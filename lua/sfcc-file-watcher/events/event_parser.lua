local M = {}

M.parse_incoming_event = function(event_str)
    local split = {}
    for str in string.gmatch(event_str, "([^".. " " .. "]+)") do
        table.insert(split, str)
    end

    local event_type = split[1]
    local file_path = split[2]

    if event_type == '0' then
        return { 'CREATE', file_path }

    elseif event_type == '1' then
        return { 'MODIFY', file_path }

    elseif event_type == '2' then
        return { 'DELETE', file_path }
    end
end

return M
