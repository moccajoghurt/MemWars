
local Colors = {}

function SetConsoleColor(color)
    local esc = string.char(27)
    os.execute("echo|set /p=" .. esc .. '[' .. color .. 'm')
end

Colors.SetConsoleColor = SetConsoleColor
Colors.red          = 31
Colors.green        = 32
Colors.white        = 37
Colors.default      = 39
Colors.black        = 90
Colors.brightred    = 91
Colors.brightgreen  = 92
Colors.magenta      = 95
Colors.cyan         = 96
Colors.brightwhite  = 97

return Colors