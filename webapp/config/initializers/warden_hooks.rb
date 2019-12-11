Warden::Manager.after_set_user do |user, auth, opts|
  scope = opts[:scope]
  auth.cookies.signed["#{scope}_id"] = user.id
  auth.cookies.signed["#{scope}_expires_at"] = 30.minutes.from_now
end

Warden::Manager.before_logout do |user, auth, opts|
  scope = opts[:scope]
  auth.cookies.signed["#{scope}_id"] = nil
  auth.cookies.signed["#{scope}_expires_at"] = nil
end
