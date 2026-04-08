
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

function runGarbageCollector()
  garbageThrottle()
  
  local start = copy(engineTime)
  for i = 1, 25 do
	  collectgarbage("step",collectionAmount)
	  if engineTime - start > 0.1 then
		  break
	  end
  end
  --if collectgarbage('count') >= 6000000 then collectgarbage("collect") end
  collectgarbage("stop")
end

function garbageThrottle()
  local garbageCount =  collectgarbage('count')

  if garbageCount > 1800 then
    collectionAmount = 25
  elseif garbageCount > 1700 then
    collectionAmount = 10
  elseif garbageCount > 1600 then
    collectionAmount = 5
  elseif garbageCount > 1500 then 
    collectionAmount = 2
  else
    collectionAmount = 1
  end
  
end