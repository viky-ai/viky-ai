json.users @users do |user|
  json.user_id user.id
  json.name user.name
  json.username user.username
  json.email user.email
  json.image user.image_url(:square)
end
