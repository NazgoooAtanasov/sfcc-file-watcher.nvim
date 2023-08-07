local M = {}

local Job = require('plenary.job')

local event_parser = require('sfcc-file-watcher.events.event_parser')
local EventHandler = require('sfcc-file-watcher.events.event_handler')

local event_handler

local function find_index(t, str)
    for idx, v in ipairs(t) do
        if v == str then
            return idx
        end
    end

    return -1
end
local plugin_path_split = vim.split(debug.getinfo(1, 'S').source, '/')
local lua_idx = find_index(plugin_path_split, "lua")
local plugin_path = '/' .. table.concat({unpack(plugin_path_split, 2, lua_idx - 1)}, '/')

local file_watch = Job:new({
    command = plugin_path .. '/file-watch/build/file-watch',
    args = {},
    cwd = vim.loop.cwd(),
    env = {},
    enable_handlers = true,
    on_start = function()
        event_handler = EventHandler:new()
        print("Event handler started")
    end,
    on_stdout = function(_err, data)
        local event = event_parser.parse_incoming_event(data)
        event_handler:accept_event(event)
    end,
    on_exit = function (_, code, _sig)
        print("Terminated " .. code)
    end,
    on_stderr = function ()
        print('there has been an error')
    end,
    skip_validation = false,
    detached = false,
    enabled_recording = false,
    writer = '',
    maximum_results = nil
});


function M.start_watch()
    file_watch:start()
end

function M.stop_watch()
    file_watch:shutdown()
end

return M
