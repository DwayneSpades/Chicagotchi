
collectionAmount = 1
function copy(o, seen)
  seen = seen or {}
  if o == nil then return nil end
  if seen[o] then return seen[o] end

  local no
  if type(o) == 'table' then
    no = {}
    seen[o] = no

    for k, v in next, o, nil do
      no[copy(k, seen)] = copy(v, seen)
    end
    setmetatable(no, copy(getmetatable(o), seen))
  else -- number, string, boolean, etc
    no = o
  end
  return no
end

function getTableSize(t)
    local count = 0
    for _, __ in pairs(t) do
        count = count + 1
    end
    return count
end

function runGarbageCollector()
  garbageThrottle()
  
  local start = engineTime
  for i = 1, 25 do
	  collectgarbage("step",collectionAmount)
	  if engineTime - start > 0.01 then
		  break
	  end
  end
  --if collectgarbage('count') >= 6000000 then collectgarbage("collect") end
  collectgarbage("stop")
end

function garbageThrottle()
  local garbageCount =  collectgarbage('count')

  if garbageCount > 120 then
    collectionAmount = 50
  elseif garbageCount > 100 then
    collectionAmount = 25
  elseif garbageCount > 80 then
    collectionAmount = 10
  elseif garbageCount > 60 then 
    collectionAmount = 5
  else
    collectionAmount = 1
  end
  
end