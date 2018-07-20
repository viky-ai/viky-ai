# Use this file to easily define all of your cron jobs.
# Learn more: http://github.com/javan/whenever

every 1.day, at: '02:00 am' do
  rake 'statistics:rollover'
end
