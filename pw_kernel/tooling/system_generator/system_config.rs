// Copyright 2025 The Pigweed Authors
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

use anyhow::{Result, anyhow};
use hashlink::LinkedHashMap;
use serde::{Deserialize, Serialize};

use crate::ArchConfigInterface;

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct SystemConfig<A: ArchConfigInterface> {
    pub arch: A,
    pub kernel: KernelConfig,
    #[serde(default)]
    pub apps: LinkedHashMap<String, AppConfig>,
    #[serde(skip_deserializing)]
    pub arch_crate_name: &'static str,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Armv8MConfig {
    #[serde(flatten)]
    pub nvic: Armv8MNvicConfig,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Armv8MNvicConfig {
    pub vector_table_start_address: u64,
    pub vector_table_size_bytes: u64,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct RiscVConfig;

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct KernelConfig {
    pub flash_start_address: u64,
    pub flash_size_bytes: u64,
    pub ram_start_address: u64,
    pub ram_size_bytes: u64,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct AppConfig {
    pub flash_size_bytes: u64,
    pub ram_size_bytes: u64,
    pub process: ProcessConfig,
    // The following fields are calculated, not defined by a user.
    // TODO: davidroth - if this becomes too un-wieldy, we should
    // split the config schema from the template structs.
    #[serde(skip_deserializing)]
    pub flash_start_address: u64,
    #[serde(skip_deserializing)]
    pub ram_start_address: u64,
    #[serde(skip_deserializing)]
    pub start_fn_address: u64,
    #[serde(skip_deserializing)]
    pub initial_sp: u64,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ProcessConfig {
    name: String,

    #[serde(default)]
    memory_mappings: LinkedHashMap<String, MemoryMapping>,

    #[serde(default)]
    objects: LinkedHashMap<String, ObjectConfig>,
    threads: Vec<ThreadConfig>,

    // Internally the template engine (`minijinja`) does not preserve the order of
    // associative containers.  Since ordering of objects in a processes object
    // table is directly related to its handle, this Vec is used to allow templates
    // to iterate over objects in order.  The same is true for memory mappings.
    //
    // `minijina` does have a `preserve-order` feature.  However, depending on
    // this can be fragile when downstream users are using non-cargo build systems
    // and managing their own third party deps.
    #[serde(skip_deserializing)]
    ordered_memory_mapping_names: Vec<String>,
    #[serde(skip_deserializing)]
    ordered_object_names: Vec<String>,
}
#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum MemoryMappingType {
    Device,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct MemoryMapping {
    #[serde(rename = "type")]
    ty: MemoryMappingType,
    start_address: u64,
    size_bytes: u64,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(tag = "type")]
#[serde(rename_all = "snake_case")]
pub enum ObjectConfig {
    Ticker(TickerConfig),
    ChannelInitiator(ChannelInitiatorConfig),
    ChannelHandler(ChannelHandlerConfig),
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct TickerConfig;

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ChannelInitiatorConfig {
    handler_app: String,
    handler_object_name: String,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ChannelHandlerConfig;

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ThreadConfig {
    name: String,
    stack_size_bytes: u64,
    priority: Option<String>,
}

impl<A: ArchConfigInterface> SystemConfig<A> {
    fn handler_exists(&self, app_name: &str, object_name: &str) -> bool {
        let Some(app) = self.apps.get(app_name) else {
            return false;
        };

        let Some(object) = app.process.objects.get(object_name) else {
            return false;
        };

        matches!(object, ObjectConfig::ChannelHandler(_))
    }

    pub fn calculate_and_validate(&mut self) -> Result<()> {
        // Generate `ordered_object_names` fields.
        for (_, app_config) in &mut self.apps {
            app_config.process.ordered_object_names = app_config
                .process
                .objects
                .iter()
                .map(|(name, _)| name.clone())
                .collect();

            app_config.process.ordered_memory_mapping_names = app_config
                .process
                .memory_mappings
                .iter()
                .map(|(name, _)| name.clone())
                .collect();
        }

        // Check to make sure that channel objects are properly linked.
        for (name, app_config) in &self.apps {
            for (ident, object) in &app_config.process.objects {
                let ObjectConfig::ChannelInitiator(initiator) = object else {
                    continue;
                };
                let handler_app = &initiator.handler_app;
                let handler_object_name = &initiator.handler_object_name;
                if !self.handler_exists(handler_app, handler_object_name) {
                    return Err(anyhow!(
                        "Channel initiator \"{name}:{ident}\" references non-existent handler \"{handler_app}\":{handler_object_name}"
                    ));
                }
            }
        }
        Ok(())
    }
}
