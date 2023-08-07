local path = require('plenary.path')
local Job = require('plenary.job')

local EventHandler = {}

local function find_index(t, str)
    for idx, v in ipairs(t) do
        if v == str then
            return idx
        end
    end

    return -1
end

local function get_webdav_path(fs_path)
    local split = vim.split(fs_path, "/")
    local cartridge_idx = find_index(split, "cartridge")

    if cartridge_idx >= 0 then
        local webdav_path = table.concat({unpack(split, cartridge_idx - 1, #split)}, "/")
        return webdav_path
    end

    return nil
end

function EventHandler:new(o)
    o = o or {}
    setmetatable(o, self)
    self.__index = self

    self:_set_config()

    return o
end

function EventHandler:_set_config()
    local found = false
    local dw_json_path = vim.loop.cwd().."/dw.json"
    local fd = path:new(dw_json_path)
    if fd:exists() then
        found = true
    end

    if found == false then
        dw_json_path = vim.loop.cwd().."/../dw.json"
        fd = path:new(dw_json_path)

        if fd:exists() then
            found = true
        end
    end

    if not fd:exists() then
        error("Cannot setup the FileWatcher because the dw.json is missing.")
    end

    local content = fd:read()
    if content ~= nil then
        local dw_json = vim.fn.json_decode(content)
        self.hostname = dw_json['hostname']
        self.username = dw_json['username']
        self.password = dw_json['password']
        self.version = dw_json['version']
        self.auth = self.username..":"..self.password
    end
end

function EventHandler:_send_file(file_path)
    local webdav_path = get_webdav_path(file_path)

    if not webdav_path then
        error("Could not convert file path to webdav file path '".. file_path .. "'")
    end

    local url = 'https://' .. self.hostname .. '/on/demandware.servlet/webdav/Sites/Cartridges/' .. self.version .. '/' .. webdav_path
    local auth = self.username .. ':' .. self.password
    local binary_data = '@' ..file_path

    vim.schedule(function ()
        local send_job = Job:new({
            command = "curl",
            args = {'-X', 'PUT', '-u', auth, '--data-binary', binary_data, url},
            env = {},
            detached = false,
            on_start = function () print("Uploading " .. webdav_path) end,
            on_exit = function () print("Uploaded " .. webdav_path) end,
            -- on_stdout = function(_, data) print(data) end
        }):start()
    end)
end

function EventHandler:_delete_file(file_path)
    local webdav_path = get_webdav_path(file_path)

    if not webdav_path then
        error("Could not convert file path to webdav file path '".. file_path .. "'")
    end

    local url = 'https://' .. self.hostname .. '/on/demandware.servlet/webdav/Sites/Cartridges/' .. self.version .. '/' .. webdav_path
    local auth = self.username .. ':' .. self.password

    vim.schedule(function ()
        local delete_job = Job:new({
            command = 'curl',
            args = {'-X', 'DELETE', '-u', auth, url},
            detached = false,
            on_start = function () print("Deleting " .. webdav_path) end,
            on_exit = function () print("Deleted " .. webdav_path) end,
            -- on_stdout = function(_, data) print(data) end
        }):start()
    end)
end

function EventHandler:accept_event(event)
    if event[1] == 'CREATE' or event[1] == 'MODIFY' then
        -- print('Uploading ' .. event[2])
        self:_send_file(event[2])

    elseif event[1] == 'DELETE' then
        -- print('Deleting ' .. event[2])
        self:_delete_file(event[2])
    end
end

return EventHandler
