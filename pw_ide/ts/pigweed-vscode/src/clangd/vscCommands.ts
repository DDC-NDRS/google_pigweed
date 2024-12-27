// Copyright 2024 The Pigweed Authors
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

import * as vscode from 'vscode';

import { ClangdActiveFilesCache } from './activeFilesCache';
import { clangdPath } from './bazel';
import { availableTargets, getTarget, targetPath } from './paths';

import { didChangeClangdConfig, didChangeTarget } from '../events';

import { launchTroubleshootingLink } from '../links';
import logger from '../logging';
import { RefreshManager } from '../refreshManager';
import { settingFor, settings, stringSettingFor } from '../settings';

export async function setTarget(
  target: string | undefined,
  settingsFileWriter: (target: string) => Promise<void>,
): Promise<void> {
  target = target ?? getTarget();
  if (!target) return;

  if (!(await availableTargets()).includes(target)) {
    throw new Error(`Target not among available targets: ${target}`);
  }

  await settings.codeAnalysisTarget(target);
  didChangeTarget.fire(target);

  const { update: updatePath } = stringSettingFor('path', 'clangd');
  const { update: updateArgs } = settingFor<string[]>('arguments', 'clangd');

  // These updates all happen asynchronously, and we want to make sure they're
  // all done before we trigger a clangd restart.
  Promise.all([
    updatePath(clangdPath()),
    updateArgs([
      `--compile-commands-dir=${targetPath(target)}`,
      '--query-driver=**',
      '--header-insertion=never',
      '--background-index',
    ]),
    settingsFileWriter(target),
  ]).then(() =>
    // Restart the clangd server so it picks up the new setting.
    vscode.commands.executeCommand('clangd.restart'),
  );
}

/** Show a checkmark next to the item if it's the current setting. */
function markIfActive(active: boolean): vscode.ThemeIcon | undefined {
  return active ? new vscode.ThemeIcon('check') : undefined;
}

export async function setCompileCommandsTarget(
  activeFilesCache: ClangdActiveFilesCache,
): Promise<void> {
  const currentTarget = getTarget();

  const targets = (await availableTargets()).sort().map((target) => ({
    label: target,
    iconPath: markIfActive(target === currentTarget),
  }));

  if (targets.length === 0) {
    vscode.window
      .showErrorMessage("Couldn't find any targets!", 'Get Help')
      .then((selection) => {
        switch (selection) {
          case 'Get Help': {
            launchTroubleshootingLink('bazel-no-targets');
            break;
          }
        }
      });

    return;
  }

  vscode.window
    .showQuickPick(targets, {
      title: 'Select a target',
      canPickMany: false,
    })
    .then(async (selection) => {
      if (!selection) return;
      const { label: target } = selection;
      await setTarget(target, activeFilesCache.writeToSettings);
    });
}

export const setCompileCommandsTargetOnSettingsChange =
  (activeFilesCache: ClangdActiveFilesCache) =>
  (e: vscode.ConfigurationChangeEvent) => {
    if (e.affectsConfiguration('pigweed')) {
      setTarget(undefined, activeFilesCache.writeToSettings);
    }
  };

export async function refreshCompileCommandsAndSetTarget(
  refresh: () => void,
  refreshManager: RefreshManager<any>,
  activeFilesCache: ClangdActiveFilesCache,
) {
  refresh();
  await refreshManager.waitFor('didRefresh');
  await setCompileCommandsTarget(activeFilesCache);
}

export async function disableInactiveFileCodeIntelligence(
  activeFilesCache: ClangdActiveFilesCache,
) {
  logger.info('Disabling inactive file code intelligence');
  await settings.disableInactiveFileCodeIntelligence(true);
  didChangeClangdConfig.fire();
  await activeFilesCache.writeToSettings(settings.codeAnalysisTarget());
  await vscode.commands.executeCommand('clangd.restart');
}

export async function enableInactiveFileCodeIntelligence(
  activeFilesCache: ClangdActiveFilesCache,
) {
  logger.info('Enabling inactive file code intelligence');
  await settings.disableInactiveFileCodeIntelligence(false);
  didChangeClangdConfig.fire();
  await activeFilesCache.writeToSettings();
  await vscode.commands.executeCommand('clangd.restart');
}