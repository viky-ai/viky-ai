require 'rainbow'

namespace :users do

  desc "Invite admin user"
  task :invite_admin, [:email] => [:environment] do |t, args|
    u = User.invite!({
      email:    args[:email],
      admin:    true
    })
    if u.errors.empty?
      puts Rainbow("\nUser will receive an email invitation").green unless Rails.env.test?
    else
      puts Rainbow("\nUser creation failed, please fix following errors:").yellow
      errors = []
      errors << u.errors.full_messages.collect{|e| "  - #{e}"}
      puts Rainbow(errors.join("\n")).yellow + "\n\n"
    end
  end

  desc "Set existing user as admin"
  task :set_admin, [:email] => [:environment] do |t, args|
    u = User.find_by_email(args[:email])
    if u.nil?
      puts Rainbow("\nSorry, user with email '#{args[:email]}' doesn't exist.\n").yellow
    else
      u.admin = true
      u.save
    end
  end

  desc "Unset existing admin user"
  task :unset_admin, [:email] => [:environment] do |t, args|
    u = User.find_by_email(args[:email])
    if u.nil?
      puts Rainbow("\nSorry, user with email '#{args[:email]}' doesn't exist.\n").yellow
    else
      u.admin = false
      u.save
    end
  end

end
