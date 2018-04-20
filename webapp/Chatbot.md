# Chatbot

## Overview
### Initiating a session
The viky.ai chatbot starts a session and sends an id token to the bot.
```
                          (send session id)
    viky.ai (chatbot) -----------------------> bot
```

### Typical chat
The user sends a statement to the bot through the chatbot interface.
The bot receives the request and process it, usually resulting in an answer or at least saying that the request has been fulfilled.
```
                         (user statement)
    viky.ai (chatbot) -----------------------> bot
       |                                        |    (interpret user statement)
       |                                        | -------------------------------> viky.ai (interpret)
       |                                        |                                     |
       |                                        | <---------------------------------- |
       |                                        |            (an intent)
       |                                        |
       |                                        |   (request to another service)
       |                                        | -------------------------------> external service
       |                                        |                                     |
       |                                        | <---------------------------------- |
       |                                        |            (answer)
       |                                        |
       | <------------------------------------- |
                      (bot answer)
```

## viky.ai
Through its public API, viky.ai exposes an endpoint to listen for bot answers.
Before any conversation can start, make sure the bot URL is configured in the bot on viky.ai.

### HTTP request
- verb : POST
- path : /api/<api_version>/chat_sessions/<session_id>/statements
```
    POST /api/<api_version>/chat_sessions/<session_id>/statements
      {
        statement: {
            nature: <nature>,
            content: {
                <...>
            }
        }
      }
```
The nature is the type of widget which will be displayed to the user. The content object changes accordingly.

### Natures
Statement natures currently available and their respective content :
- text :
```
      {
        statement: {
            nature: 'text',
            content: {
                text: <text>
            }
        }
      }
```

## Bot
A bot is a remote service implementing the desired business logic.
It is reachable through a REST API and it must include those following endpoints in order to work with the viky.ai chatbot :

### Endpoints
- Listen for new chat session :
    * verb : POST
    * path : /start
```
    POST /start
      {
          session_id: <id>
      }
```
When a user starts a conversion from viky.ai, the bot will receive a new session id.
The bot must keep this session id if it wants to follow the context of several conversations at the same time.
For a specific bot, only the last session is valid and any request to a closed one will result in a `403 Forbidden` response.

- Listen for a new user statement :
    * verb : POST
    * path : /sessions/<session_id>/user_statements
```
    POST /sessions/<session_id>/user_statements
      {
          user_statement: {
              says: <text>,
          }
      }
```
A user mades a new statement in the conversation. It must include the current session id as URL parameter.

- Bot is alive :
    * verb : GET
    * path : /ping
```
    GET /ping
```
viky.ai wants to know if the bot is up and running mainly to display a status indicator for the user.
It expects an HTTP code `200 OK` if the bot is available. Any other code (or the lack of answer) will result in considering the bot is unavailable. 
