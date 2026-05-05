--runs all the active alarms and checks their times


alarmManager = {}
alarmManager.__index=alarmManager
alarmManager.activeAlarms = {}

--example dictionary structure
function alarmManager:addAlarm(a,key)
  self.activeAlarms[key] = a
end

function alarmManager:removeAlarm(key)
  self.activeAlarms[key] = nil
end

function alarmManager:update()
  --NOTE: ipairs and pairs are different!!! 
  --pairs can loop over arbitrarily named key value pairs while ipairs can iterate over value entries that have no key tied to them
  
  for i,v in pairs(self.activeAlarms) do
    v:checkAlarm()
  end
  
end
