﻿// Copyright (c) Microsoft Corporation
// The Microsoft Corporation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.IO.Abstractions;
using System.Linq;
using Newtonsoft.Json;

namespace Wox.Plugin
{
    [JsonObject(MemberSerialization.OptOut)]
    public class PluginMetadata : BaseModel
    {
        private static readonly IFileSystem FileSystem = new FileSystem();
        private static readonly IPath Path = FileSystem.Path;

        private string _pluginDirectory;

        public PluginMetadata()
        {
        }

        public string ID { get; set; }

        public string Name { get; set; }

        public string Author { get; set; }

        public string Version { get; set; }

        public string Language { get; set; }

        public string Website { get; set; }

        public bool Disabled { get; set; }

        public string ExecuteFilePath { get; private set; }

        public string ExecuteFileName { get; set; }

        public string PluginDirectory
        {
            get
            {
                return _pluginDirectory;
            }

            internal set
            {
                _pluginDirectory = value;
                ExecuteFilePath = Path.Combine(value, ExecuteFileName);
                IcoPathDark = Path.Combine(value, IcoPathDark);
                IcoPathLight = Path.Combine(value, IcoPathLight);
            }
        }

        public string ActionKeyword { get; set; }

        public bool IsGlobal { get; set; }

        public string Glyph { get; set; }

        public string IcoPathDark { get; set; }

        public string IcoPathLight { get; set; }

        public override string ToString()
        {
            return Name;
        }

        /// <summary>
        /// Gets or sets init time include both plugin load time and init time
        /// </summary>
        [JsonIgnore]
        public long InitTime { get; set; }

        [JsonIgnore]
        public long AvgQueryTime { get; set; }

        [JsonIgnore]
        public int QueryCount { get; set; }
    }
}
