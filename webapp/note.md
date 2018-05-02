

# Entry
# * bot_id
# * user_id
# * from : bot|user
# * type :
# * content : JSON serialisé

# Create dialog
# POST "http://www.mybot/api/v1/conversation/"

# POST "http://www.mybot/api/v1/conversation/"
# {
#   session_id: "dsqdsqdq"
# }

# POST endpoint
# {
#   chatbot_id: "bot_uuid--user_uuid"
# }

# Controller API
#
# POST viky-beta/api/v1/chatbot/chatbot_id/entries
# chatbot_id -> user & agent bot

# params : {
#   entry: {
#     type: "text"
#     data: {
#       ...
#     }
#   }
# }
# def create
#   if entry.save
#     render json: [] #JSON pour Bot
#   end
# end

# # Model Entry
# class Entry
#   after_create :update_chatbot_ui #, if: -> {|e| e.type == "bot" }

#   def to_html
#     case type
#     when 'text'
#       html = render partial ...
#     end
#     html
#   end

#   def update_chatbot_ui
#     html = self.to_html
#     ChatbotNotificationsChannel.broadcast_to [current_user, chatbot], html: @html
#   end
# end

# # Vue Chatbot
# <% entrie.each do |entry| %>
#   <%= entry.to_html %>
# <% end %>
