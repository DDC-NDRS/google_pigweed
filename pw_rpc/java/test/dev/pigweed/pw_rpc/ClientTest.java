// Copyright 2021 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

package dev.pigweed.pw_rpc;

import static com.google.common.truth.Truth.assertThat;
import static java.nio.charset.StandardCharsets.UTF_8;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import com.google.common.collect.ImmutableList;
import com.google.protobuf.ExtensionRegistryLite;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.MessageLite;
import dev.pigweed.pw_rpc.internal.Packet.PacketType;
import dev.pigweed.pw_rpc.internal.Packet.RpcPacket;
import java.util.ArrayList;
import java.util.List;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

public final class ClientTest {
  @Rule public final MockitoRule mockito = MockitoJUnit.rule();

  private static final Service SERVICE = new Service("pw.rpc.test1.TheTestService",
      Service.unaryMethod("SomeUnary", SomeMessage.parser(), AnotherMessage.parser()),
      Service.serverStreamingMethod(
          "SomeServerStreaming", SomeMessage.parser(), AnotherMessage.parser()),
      Service.clientStreamingMethod(
          "SomeClientStreaming", SomeMessage.parser(), AnotherMessage.parser()),
      Service.bidirectionalStreamingMethod(
          "SomeBidiStreaming", SomeMessage.parser(), AnotherMessage.parser()));

  private static final Method UNARY_METHOD = SERVICE.method("SomeUnary");
  private static final Method SERVER_STREAMING_METHOD = SERVICE.method("SomeServerStreaming");
  private static final Method CLIENT_STREAMING_METHOD = SERVICE.method("SomeClientStreaming");

  private static final int CHANNEL_ID = 1;

  private static final SomeMessage REQUEST_PAYLOAD =
      SomeMessage.newBuilder().setMagicNumber(54321).build();
  private static final AnotherMessage RESPONSE_PAYLOAD =
      AnotherMessage.newBuilder()
          .setResult(AnotherMessage.Result.FAILED_MISERABLY)
          .setPayload("12345")
          .build();

  private Client client;
  private Client legacyClient;
  private List<RpcPacket> packetsSent;

  @Mock private StreamObserver<AnotherMessage> observer;

  private static byte[] response(String service, String method) {
    return response(service, method, Status.OK);
  }

  private static byte[] response(String service, String method, Status status) {
    return serverReply(
        PacketType.RESPONSE, service, method, status, SomeMessage.getDefaultInstance());
  }

  private static byte[] response(
      String service, String method, Status status, MessageLite payload) {
    return serverReply(PacketType.RESPONSE, service, method, status, payload);
  }

  private static byte[] legacyResponse(
      String service, String method, Status status, MessageLite payload) {
    return responseWithCallId(service, method, status, payload, 0);
  }

  private static byte[] responseWithCallId(
      String service, String method, Status status, MessageLite payload, int callId) {
    return serverReplyWithCallId(PacketType.RESPONSE, service, method, status, payload, callId);
  }

  private static byte[] serverStream(String service, String method, MessageLite payload) {
    return serverReply(PacketType.SERVER_STREAM, service, method, Status.OK, payload);
  }

  private static byte[] serverStreamWithCallId(
      String service, String method, MessageLite payload, int callId) {
    return serverReplyWithCallId(
        PacketType.SERVER_STREAM, service, method, Status.OK, payload, callId);
  }

  private static byte[] legacyServerStream(String service, String method, MessageLite payload) {
    return serverStreamWithCallId(service, method, payload, 0);
  }

  private static byte[] serverReply(
      PacketType type, String service, String method, Status status, MessageLite payload) {
    return packetBuilder(service, method)
        .setType(type)
        .setStatus(status.code())
        .setPayload(payload.toByteString())
        .build()
        .toByteArray();
  }

  private static byte[] serverReplyWithCallId(PacketType type,
      String service,
      String method,
      Status status,
      MessageLite payload,
      int callId) {
    return packetBuilder(service, method)
        .setCallId(callId)
        .setType(type)
        .setStatus(status.code())
        .setPayload(payload.toByteString())
        .build()
        .toByteArray();
  }

  private static RpcPacket.Builder packetBuilder(String service, String method) {
    return packetBuilderWithCallId(service, method, Endpoint.FIRST_CALL_ID);
  }

  private static RpcPacket.Builder packetBuilderWithCallId(
      String service, String method, int callId) {
    return RpcPacket.newBuilder()
        .setChannelId(CHANNEL_ID)
        .setCallId(callId)
        .setServiceId(Ids.calculate(service))
        .setMethodId(Ids.calculate(method));
  }

  private static RpcPacket requestPacketWithCallId(
      String service, String method, MessageLite payload, int callId) {
    return packetBuilderWithCallId(service, method, callId)
        .setType(PacketType.REQUEST)
        .setPayload(payload.toByteString())
        .build();
  }

  private static RpcPacket responsePacketWithCallId(
      String service, String method, MessageLite payload, int callId) {
    return packetBuilderWithCallId(service, method, callId)
        .setType(PacketType.RESPONSE)
        .setPayload(payload.toByteString())
        .build();
  }

  private static RpcPacket legacyRequestPacket(String service, String method, MessageLite payload) {
    return requestPacketWithCallId(service, method, payload, 0);
  }

  private static RpcPacket requestPacket(String service, String method, MessageLite payload) {
    return packetBuilder(service, method)
        .setType(PacketType.REQUEST)
        .setPayload(payload.toByteString())
        .build();
  }

  @Before
  public void setup() {
    packetsSent = new ArrayList<>();
    Channel channel = new Channel(CHANNEL_ID, (data) -> {
      try {
        packetsSent.add(RpcPacket.parseFrom(data, ExtensionRegistryLite.getEmptyRegistry()));
      } catch (InvalidProtocolBufferException e) {
        fail("The client sent an invalid packet: " + e);
      }
    });

    client = Client.createMultiCall(ImmutableList.of(channel), ImmutableList.of(SERVICE));
    legacyClient =
        Client.createLegacySingleCall(ImmutableList.of(channel), ImmutableList.of(SERVICE));
  }

  @Test
  public void method_invalidFormat() {
    assertThrows(IllegalArgumentException.class, () -> client.method(CHANNEL_ID, ""));
    assertThrows(IllegalArgumentException.class, () -> client.method(CHANNEL_ID, "one"));
    assertThrows(IllegalArgumentException.class, () -> client.method(CHANNEL_ID, "hello"));
  }

  @Test
  public void method_unknownService() {
    assertThrows(
        InvalidRpcServiceException.class, () -> client.method(CHANNEL_ID, "abc.Service/Method"));

    Service service = new Service("throwaway.NotRealService",
        Service.unaryMethod("NotAnRpc", SomeMessage.parser(), AnotherMessage.parser()));
    assertThrows(InvalidRpcServiceException.class,
        () -> client.method(CHANNEL_ID, service.method("NotAnRpc")));
  }

  @Test
  public void method_unknownMethodInKnownService() {
    assertThrows(InvalidRpcServiceMethodException.class,
        () -> client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService/NotAnRpc"));
    assertThrows(InvalidRpcServiceMethodException.class,
        () -> client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "NotAnRpc"));
  }

  @Test
  public void method_unknownChannel() {
    MethodClient methodClient0 = client.method(0, "pw.rpc.test1.TheTestService/SomeUnary");
    assertThrows(InvalidRpcChannelException.class,
        () -> methodClient0.invokeUnary(SomeMessage.getDefaultInstance()));

    MethodClient methodClient999 = client.method(999, "pw.rpc.test1.TheTestService/SomeUnary");
    assertThrows(InvalidRpcChannelException.class,
        () -> methodClient999.invokeUnary(SomeMessage.getDefaultInstance()));
  }

  @Test
  public void method_accessAsServiceSlashMethod() {
    assertThat(client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService/SomeUnary").method())
        .isSameInstanceAs(UNARY_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService/SomeServerStreaming").method())
        .isSameInstanceAs(SERVER_STREAMING_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService/SomeClientStreaming").method())
        .isSameInstanceAs(CLIENT_STREAMING_METHOD);
  }

  @Test
  public void method_accessAsServiceDotMethod() {
    assertThat(client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService.SomeUnary").method())
        .isSameInstanceAs(UNARY_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService.SomeServerStreaming").method())
        .isSameInstanceAs(SERVER_STREAMING_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService.SomeClientStreaming").method())
        .isSameInstanceAs(CLIENT_STREAMING_METHOD);
  }

  @Test
  public void method_accessAsServiceAndMethod() {
    assertThat(client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary").method())
        .isSameInstanceAs(UNARY_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming").method())
        .isSameInstanceAs(SERVER_STREAMING_METHOD);
    assertThat(
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeClientStreaming").method())
        .isSameInstanceAs(CLIENT_STREAMING_METHOD);
  }

  @Test
  public void method_accessFromMethodInstance() {
    assertThat(client.method(CHANNEL_ID, UNARY_METHOD).method()).isSameInstanceAs(UNARY_METHOD);
    assertThat(client.method(CHANNEL_ID, SERVER_STREAMING_METHOD).method())
        .isSameInstanceAs(SERVER_STREAMING_METHOD);
    assertThat(client.method(CHANNEL_ID, CLIENT_STREAMING_METHOD).method())
        .isSameInstanceAs(CLIENT_STREAMING_METHOD);
  }

  @Test
  public void processPacket_emptyPacket_isNotProcessed() {
    assertThat(client.processPacket(new byte[] {})).isFalse();
  }

  @Test
  public void processPacket_invalidPacket_isNotProcessed() {
    assertThat(client.processPacket("\uffff\uffff\uffffNot a packet!".getBytes(UTF_8))).isFalse();
  }

  @Test
  public void processPacket_packetNotForClient_isNotProcessed() {
    assertThat(client.processPacket(RpcPacket.newBuilder()
                       .setType(PacketType.REQUEST)
                       .setChannelId(CHANNEL_ID)
                       .setServiceId(123)
                       .setMethodId(456)
                       .build()
                       .toByteArray()))
        .isFalse();
  }

  @Test
  public void processPacket_unrecognizedChannel_isNotProcessed() {
    assertThat(client.processPacket(RpcPacket.newBuilder()
                       .setType(PacketType.RESPONSE)
                       .setChannelId(CHANNEL_ID + 100)
                       .setServiceId(123)
                       .setMethodId(456)
                       .build()
                       .toByteArray()))
        .isFalse();
  }

  @Test
  public void processPacket_unrecognizedService_sendsError() {
    assertThat(client.processPacket(response("pw.rpc.test1.NotAService", "SomeUnary"))).isTrue();

    assertThat(packetsSent)
        .containsExactly(packetBuilder("pw.rpc.test1.NotAService", "SomeUnary")
                .setType(PacketType.CLIENT_ERROR)
                .setStatus(Status.NOT_FOUND.code())
                .build());
  }

  @Test
  public void processPacket_unrecognizedMethod_sendsError() {
    assertThat(client.processPacket(response("pw.rpc.test1.TheTestService", "NotMethod"))).isTrue();

    assertThat(packetsSent)
        .containsExactly(packetBuilder("pw.rpc.test1.TheTestService", "NotMethod")
                .setType(PacketType.CLIENT_ERROR)
                .setStatus(Status.NOT_FOUND.code())
                .build());
  }

  @Test
  public void processPacket_nonPendingMethod_sendsError() {
    assertThat(client.processPacket(response("pw.rpc.test1.TheTestService", "SomeUnary"))).isTrue();

    assertThat(packetsSent)
        .containsExactly(packetBuilder("pw.rpc.test1.TheTestService", "SomeUnary")
                .setType(PacketType.CLIENT_ERROR)
                .setStatus(Status.FAILED_PRECONDITION.code())
                .build());
  }

  @Test
  public void processPacket_serverError_abortsPending() throws Exception {
    MethodClient method = client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary");
    Call call = method.invokeUnary(SomeMessage.getDefaultInstance());

    assertThat(client.processPacket(serverReply(PacketType.SERVER_ERROR,
                   "pw.rpc.test1.TheTestService",
                   "SomeUnary",
                   Status.NOT_FOUND,
                   SomeMessage.getDefaultInstance())))
        .isTrue();
    assertThat(call.error()).isEqualTo(Status.NOT_FOUND);
  }

  @Test
  public void processPacket_responseToPendingUnaryMethod_callsObserver() throws Exception {
    MethodClient method = client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary");

    method.invokeUnary(REQUEST_PAYLOAD, observer);

    assertThat(packetsSent)
        .containsExactly(
            requestPacket("pw.rpc.test1.TheTestService", "SomeUnary", REQUEST_PAYLOAD));

    assertThat(
        client.processPacket(response(
            "pw.rpc.test1.TheTestService", "SomeUnary", Status.ALREADY_EXISTS, RESPONSE_PAYLOAD)))
        .isTrue();

    verify(observer).onNext(RESPONSE_PAYLOAD);
    verify(observer).onCompleted(Status.ALREADY_EXISTS);
  }

  @Test
  public void processPacket_responsesToPendingServerStreamingMethod_callsObserver()
      throws Exception {
    MethodClient method =
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    method.invokeServerStreaming(REQUEST_PAYLOAD, observer);

    assertThat(packetsSent)
        .containsExactly(
            requestPacket("pw.rpc.test1.TheTestService", "SomeServerStreaming", REQUEST_PAYLOAD));

    assertThat(client.processPacket(serverStream(
                   "pw.rpc.test1.TheTestService", "SomeServerStreaming", RESPONSE_PAYLOAD)))
        .isTrue();

    verify(observer).onNext(RESPONSE_PAYLOAD);

    assertThat(client.processPacket(response(
                   "pw.rpc.test1.TheTestService", "SomeServerStreaming", Status.UNAUTHENTICATED)))
        .isTrue();

    verify(observer).onCompleted(Status.UNAUTHENTICATED);
  }

  @Test
  public void processPacket_responsePacket_completesRpc() throws Exception {
    MethodClient method =
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    method.invokeServerStreaming(REQUEST_PAYLOAD, observer);

    assertThat(client.processPacket(
                   response("pw.rpc.test1.TheTestService", "SomeServerStreaming", Status.OK)))
        .isTrue();

    verify(observer).onCompleted(Status.OK);

    assertThat(client.processPacket(serverStream(
                   "pw.rpc.test1.TheTestService", "SomeServerStreaming", RESPONSE_PAYLOAD)))
        .isTrue();

    verify(observer, never()).onNext(any());
  }

  @Test
  public void legacyProcessPacket_unary_callsObserver() throws Exception {
    MethodClient method =
        legacyClient.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary");

    method.invokeUnary(REQUEST_PAYLOAD, observer);

    assertThat(packetsSent)
        .containsExactly(
            legacyRequestPacket("pw.rpc.test1.TheTestService", "SomeUnary", REQUEST_PAYLOAD));

    assertThat(
        legacyClient.processPacket(legacyResponse(
            "pw.rpc.test1.TheTestService", "SomeUnary", Status.ALREADY_EXISTS, RESPONSE_PAYLOAD)))
        .isTrue();

    verify(observer).onNext(RESPONSE_PAYLOAD);
    verify(observer).onCompleted(Status.ALREADY_EXISTS);
  }

  @Test
  public void legacyProcessPacket_streaming_callsObserver() throws Exception {
    MethodClient method =
        legacyClient.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    method.invokeServerStreaming(REQUEST_PAYLOAD, observer);

    assertThat(legacyClient.processPacket(legacyResponse("pw.rpc.test1.TheTestService",
                   "SomeServerStreaming",
                   Status.OK,
                   AnotherMessage.getDefaultInstance())))
        .isTrue();

    verify(observer).onCompleted(Status.OK);

    assertThat(legacyClient.processPacket(legacyServerStream(
                   "pw.rpc.test1.TheTestService", "SomeServerStreaming", RESPONSE_PAYLOAD)))
        .isTrue();

    verify(observer, never()).onNext(any());
  }

  @Test
  public void processPacket_unaryOpenCallId_callsObserver() throws Exception {
    MethodClient method = client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary");

    method.invokeUnary(REQUEST_PAYLOAD, observer);

    assertThat(packetsSent)
        .containsExactly(
            requestPacket("pw.rpc.test1.TheTestService", "SomeUnary", REQUEST_PAYLOAD));

    assertThat(client.processPacket(responseWithCallId("pw.rpc.test1.TheTestService",
                   "SomeUnary",
                   Status.ALREADY_EXISTS,
                   RESPONSE_PAYLOAD,
                   0)))
        .isTrue();

    verify(observer).onNext(RESPONSE_PAYLOAD);
    verify(observer).onCompleted(Status.ALREADY_EXISTS);
  }

  @Test
  public void processPacket_streamingOpenCallId_callsObserver() throws Exception {
    MethodClient method =
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    method.invokeServerStreaming(REQUEST_PAYLOAD, observer);

    assertThat(client.processPacket(responseWithCallId("pw.rpc.test1.TheTestService",
                   "SomeServerStreaming",
                   Status.OK,
                   AnotherMessage.getDefaultInstance(),
                   Endpoint.OPEN_CALL_ID)))
        .isTrue();

    verify(observer).onCompleted(Status.OK);

    assertThat(client.processPacket(serverStreamWithCallId("pw.rpc.test1.TheTestService",
                   "SomeServerStreaming",
                   RESPONSE_PAYLOAD,
                   Endpoint.OPEN_CALL_ID)))
        .isTrue();

    verify(observer, never()).onNext(any());
  }

  @Test
  public void processPacket_openUnaryMethod_callsObserver() throws Exception {
    MethodClient method = client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary");

    method.openUnary(observer);

    assertThat(client.processPacket(responseWithCallId("pw.rpc.test1.TheTestService",
                   "SomeUnary",
                   Status.OK,
                   RESPONSE_PAYLOAD,
                   Endpoint.OPEN_CALL_ID)))
        .isTrue();

    verify(observer).onCompleted(Status.OK);
  }

  @Test
  public void processPacket_openStreamingMethod_callsObserver() throws Exception {
    MethodClient method =
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    method.openServerStreaming(observer);

    assertThat(client.processPacket(serverStreamWithCallId("pw.rpc.test1.TheTestService",
                   "SomeServerStreaming",
                   RESPONSE_PAYLOAD,
                   Endpoint.OPEN_CALL_ID)))
        .isTrue();

    verify(observer).onNext(RESPONSE_PAYLOAD);
    verify(observer, never()).onCompleted(any());

    assertThat(client.processPacket(responseWithCallId("pw.rpc.test1.TheTestService",
                   "SomeServerStreaming",
                   Status.OK,
                   AnotherMessage.getDefaultInstance(),
                   Endpoint.OPEN_CALL_ID)))
        .isTrue();

    verify(observer).onCompleted(Status.OK);
  }

  @Test
  @SuppressWarnings("unchecked")
  public void streamObserverClient_create_invokeMethod() throws Exception {
    Channel.Output mockChannelOutput = Mockito.mock(Channel.Output.class);
    Client client = Client.createMultiCall(ImmutableList.of(new Channel(1, mockChannelOutput)),
        ImmutableList.of(SERVICE),
        (rpc) -> Mockito.mock(StreamObserver.class));

    SomeMessage payload = SomeMessage.newBuilder().setMagicNumber(99).build();
    client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeUnary").invokeUnary(payload);
    verify(mockChannelOutput)
        .send(requestPacket("pw.rpc.test1.TheTestService", "SomeUnary", payload).toByteArray());
  }

  @Test
  public void closeChannel_abortsExisting() throws Exception {
    MethodClient serverStreamMethod =
        client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeServerStreaming");

    Call call1 = serverStreamMethod.invokeServerStreaming(REQUEST_PAYLOAD, observer);
    Call call2 = client.method(CHANNEL_ID, "pw.rpc.test1.TheTestService", "SomeClientStreaming")
                     .invokeClientStreaming(observer);
    assertThat(call1.active()).isTrue();
    assertThat(call2.active()).isTrue();

    assertThat(client.closeChannel(CHANNEL_ID)).isTrue();

    assertThat(call1.active()).isFalse();
    assertThat(call2.active()).isFalse();

    verify(observer, times(2)).onError(Status.ABORTED);

    assertThrows(InvalidRpcChannelException.class,
        () -> serverStreamMethod.invokeServerStreaming(REQUEST_PAYLOAD, observer));
  }

  @Test
  public void closeChannel_noCalls() {
    assertThat(client.closeChannel(CHANNEL_ID)).isTrue();
  }

  @Test
  public void closeChannel_knownChannel() {
    assertThat(client.closeChannel(CHANNEL_ID + 100)).isFalse();
  }

  @Test
  public void openChannel_uniqueChannel() throws Exception {
    int newChannelId = CHANNEL_ID + 100;
    Channel.Output channelOutput = Mockito.mock(Channel.Output.class);
    client.openChannel(new Channel(newChannelId, channelOutput));

    client.method(newChannelId, "pw.rpc.test1.TheTestService", "SomeUnary")
        .invokeUnary(REQUEST_PAYLOAD, observer);

    verify(channelOutput)
        .send(requestPacket("pw.rpc.test1.TheTestService", "SomeUnary", REQUEST_PAYLOAD)
                .toBuilder()
                .setChannelId(newChannelId)
                .build()
                .toByteArray());
  }

  @Test
  public void openChannel_alreadyExists_throwsException() {
    assertThrows(InvalidRpcChannelException.class,
        () -> client.openChannel(new Channel(CHANNEL_ID, packet -> {})));
  }
}
