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

import * as path from 'path';

import { loadLegacySettings, loadLegacySettingsFile } from '../settings/legacy';
import { settings, workingDir } from '../settings/vscode';
import { globStream } from 'glob';
import { CDB_FILE_NAME } from './paths';
import logger from '../logging';
import {
  CompilationDatabase,
  CompilationDatabaseMap,
  inferTarget,
} from './parser';

interface CompDbProcessingSettings {
  compDbSearchPaths: string[][];
  workingDir: string;
}

/**
 * Get user settings related to compilation database processing, from the
 * VS Code settings (canonical source) or the legacy `.pw_ide.yaml`.
 */
async function getCompDbProcessingSettings(): Promise<CompDbProcessingSettings> {
  // If this returns null, we assume there is no legacy settings file.
  const legacySettingsData = await loadLegacySettingsFile();

  // If there is a legacy settings file, assume all relevant config is there.
  if (legacySettingsData !== null) {
    logger.info('Using legacy settings file: .pw_ide.yaml');

    const legacySettings = await loadLegacySettings(
      legacySettingsData ?? '',
      true,
    );

    return {
      compDbSearchPaths: legacySettings.compdb_search_paths,
      workingDir: legacySettings.working_dir,
    };
  }

  // Otherwise use the values from the VS Code config or the defaults.
  return {
    compDbSearchPaths: settings
      .compDbSearchPaths()
      .map(({ pathGlob, targetInferencePattern }) => [
        pathGlob,
        targetInferencePattern,
      ]),
    workingDir: workingDir.get(),
  };
}

/**
 * An async generator that finds compilation database files based on provided
 * search path globs.
 *
 * For example, a common search path glob for GN projects would be `out/*`.
 * That would expand to every top-level directory in `out`. Then, each of those
 * directories will be recursively searched for compilation databases.
 *
 * The return value is a tuple of the specific (expanded) search path that
 * yielded the compilation database, the path to the compilation database,
 * and the target inference pattern that was associated with the original
 * search path glob.
 */
async function* assembleCompDbFileData(
  compDbSearchPaths: string[][],
  workingDir: string,
) {
  for (const [
    baseSearchPathGlob,
    searchPathTargetInference,
  ] of compDbSearchPaths) {
    const searchPathGlob = path.join(workingDir, baseSearchPathGlob);

    // For each search path glob, get an array of concrete directory paths.
    for await (const searchPath of globStream(searchPathGlob)) {
      // For each directory path, get an array of compDb file paths.
      for await (const compDbFilePath of globStream(
        `${searchPath}/${CDB_FILE_NAME}`,
      )) {
        // Associate each compDb file path with its root directory and target
        // inference pattern.
        yield [searchPath, compDbFilePath, searchPathTargetInference];
      }
    }
  }
}

/**
 * Process compilation databases found in the search path globs in settings.
 *
 * This returns two things:
 *
 * 1. A compilation database map that associates target names (generated from
 * target inference) with compilation database objects containing compile
 * commands only for that target. These should be written to disk in the
 * canonical compile commands directory.
 *
 * 2. A collection of compilation databases that don't require processing,
 * because they already contain compile commands only for a single target. These
 * include only the target name and the path to the compilation database, so
 * the IDE can be configured to point directly at the source files.
 */
export async function processCompDbs() {
  logger.info('Processing compilation databases...');

  const { compDbSearchPaths, workingDir } = await getCompDbProcessingSettings();

  const unprocessedCompDbs: [string, string][] = [];
  const processedCompDbMaps: CompilationDatabaseMap[] = [];
  let fileCount = 0;

  for await (const [
    searchPath,
    compDbFilePath,
    searchPathTargetInference,
  ] of assembleCompDbFileData(compDbSearchPaths, workingDir)) {
    const compDb = await CompilationDatabase.fromFile(compDbFilePath, () =>
      logger.error('bad file'),
    );

    if (!compDb) continue;

    const processed = compDb.process(searchPathTargetInference);
    fileCount++;

    if (!processed) {
      const outputPath = path.dirname(
        path.relative(searchPath, compDbFilePath),
      );
      const targetName = inferTarget(searchPathTargetInference, outputPath);
      unprocessedCompDbs.push([targetName, compDbFilePath]);
    } else {
      processedCompDbMaps.push(processed);
    }
  }

  const processedCompDbs = CompilationDatabaseMap.merge(...processedCompDbMaps);

  logger.info(`↳ Processed ${fileCount} files`);
  logger.info(
    `↳ Found ${unprocessedCompDbs.length} compilation databases that can be used in place`,
  );
  logger.info(
    `↳ Produced ${processedCompDbs.size} clean compilation databases`,
  );
  logger.info('');

  return {
    processedCompDbs,
    unprocessedCompDbs,
  };
}
