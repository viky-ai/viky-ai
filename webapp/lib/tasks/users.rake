require_relative 'lib/task'

namespace :users do

  if Feature.email_configured?
    desc "Invite admin user"
    task :invite_admin, [:email] => [:environment] do |t, args|
      u = User.invite!({
        email:    args[:email],
        admin:    true
      })
      if u.errors.empty?
        Task::Print.success("User will receive an email invitation") unless Rails.env.test?
      else
        Task::Print.error("User creation failed, please fix following errors:")
        u.errors.full_messages.each do |msg|
          Task::Print.warning("  - #{msg}")
        end
      end
    end
  end

  desc "Set existing user as admin"
  task :set_admin, [:email] => [:environment] do |t, args|
    u = User.find_by_email(args[:email])
    if u.nil?
      Task::Print.error("Sorry, user with email '#{args[:email]}' doesn't exist.")
    else
      u.admin = true
      u.save
    end
  end

  desc "Unset existing admin user"
  task :unset_admin, [:email] => [:environment] do |t, args|
    u = User.find_by_email(args[:email])
    if u.nil?
      Task::Print.error("Sorry, user with email '#{args[:email]}' doesn't exist.\n")
    else
      u.admin = false
      u.save
    end
  end

end
