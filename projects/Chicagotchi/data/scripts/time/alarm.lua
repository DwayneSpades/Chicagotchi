alarm={}
alarm.__index = alarm
alarmCount = 0
function alarm.new(duration,ev)
  local instance = setmetatable({}, alarm)
  instance.alarmTime = (engineTime + duration)
  
  --add to the maanger automatically to be handled and triggered in an update loop
  
  instance.key = "alarm_"..alarmCount
  alarmCount = alarmCount + 1
  instance.event = ev or nil
  alarmManager:addAlarm(instance,instance.key)
  
  return instance
  
end

function alarm:destroy()
  alarmManager:removeAlarm(self.key)
  --self = nil
end

function alarm:checkAlarm()

  if(self.alarmTime <= engineTime) then
    --trigger the event
    if self.event ~= nil then
      self:event()
    end
    self:destroy()
  end
end