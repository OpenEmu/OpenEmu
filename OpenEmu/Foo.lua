keys = {
    ["up"] = 0,
    ["down"] = 1,
    ["left"] = 2,
    ["right"] = 3,
    ["a"] = 4,
    ["b"] = 5,
    ["start"] = 6,
    ["select"] = 7,
}

ticks = 0
stateIdx = 1
-- count, up, down, left, right, a, b, start, select
states = {
    {100, 0, 0, 0, 0, 0, 0, 0, 0},
    { 10, 0, 0, 0, 0, 0, 0, 1, 0},
    {100, 0, 0, 0, 0, 0, 0, 0, 0},
    {150, 0, 0, 0, 1, 0, 1, 0, 0},
    { 10, 0, 0, 0, 1, 1, 1, 0, 0},
    {100, 0, 0, 0, 1, 0, 1, 0, 0},
}

prevState = states[1]
curState = states[1]

function HandleKey(game_core, prev, cur, key)
    if not (prev == cur) then
        if cur == 1 then
            game_core:didPushNESButton_forPlayer_(key, 1)
        else
            game_core:didReleaseNESButton_forPlayer_(key, 1)
        end
    end
end

function OnFrameTick(game_core) 
    HandleKey(game_core, prevState[2], curState[2], 0)
    HandleKey(game_core, prevState[3], curState[3], 1)
    HandleKey(game_core, prevState[4], curState[4], 2)
    HandleKey(game_core, prevState[5], curState[5], 3)
    HandleKey(game_core, prevState[6], curState[6], 4)
    HandleKey(game_core, prevState[7], curState[7], 5)
    HandleKey(game_core, prevState[8], curState[8], 6)
    HandleKey(game_core, prevState[9], curState[9], 7)
    
    prevState = states[stateIdx]
    if curState[1] == 0 then
        stateIdx = stateIdx + 1
        curState = states[stateIdx]
    else
        curState[1] = curState[1] - 1
    end
end


function OnGameLoaded(game_core)
    print("OnGameLoaded", game_core)
end