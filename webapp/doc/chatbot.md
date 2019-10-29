# Chatbot

## Overview

## Enable chatbot feature

The chatbot feature can be activated using the environment variable `VIKYAPP_CHATBOT_ENABLED`. This feature is disabled by default. When activated, chatbot can be activated per user in order to allow chatbot feature for a well-defined list of users.

### Initiating a session

The viky.ai chatbot starts a session and sends an id token to the bot.

```
                      (send session id)
viky.ai (chatbot) -----------------------> bot
```

### Typical chat interactions

The user sends a statement to the bot through the chatbot interface.
The bot receives the request and process it, usually resulting in an answer or at least saying that the request has been fulfilled.

```
                   (user action)
viky.ai (chatbot) -----------------> bot
   |                                  |    (interpret user action)
   |                                  | -----------------------------> viky.ai (interpret)
   |                                  |                                   |
   |                                  | <-------------------------------- |
   |                                  |            (an intent)
   |                                  |
   |                                  |   (request another service)
   |                                  | -----------------------------> external service
   |                                  |                                   |
   |                                  | <-------------------------------- |
   |                                  |            (answer)
   |                                  |
   | <------------------------------- |
                   (bot answer)
```

## API conventions

The API is based on the HTTP protocol.

The settings must be encoded in UTF-8 and respect the URL encoding.

The return in UTF-8 uses the JSON format.

Using APIs requires the Header HTTP `Accept: application/json` and the Header HTTP `Content-Type: application/json` to be systematically sent when the method used requires the sending of JSONs in the HTTP body.

## viky.ai API

Through its public API, viky.ai exposes endpoint to listen for bot answers.
Before any conversation can start, make sure the bot URL is configured in the bot on viky.ai.

### Create statement

#### Endpoint

`POST /api/v1/chat_sessions/<session_id>/statements`

#### JSON
```
 {
   statement: {
     nature: <nature>,
     content: {
      ...,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
     }
   }
 }
```

The nature (text, image, map ...) is the type of widget which will be displayed to the user. The content object changes accordingly.

<code>speech</code> parameter is available in all statement natures.

* `<speech_text>` Text to speech (via text to speech).
* `<speech_locale>` Locale of the text to speech.

**Note:** Available speech locales are `ru-RU`, `ar`, `ja-JP`, `ko-KR`, `zh`, `en-US`, `en-GB`, `fr-FR`, `es-ES`, `it-IT` and `de-DE`.

#### <code>text</code> nature

Display text, URLs are automatically converted to HTML links.

JSON structure :
```
{
  statement: {
    nature: 'text',
    content: {
      text: <text>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `<text>` Text to display. Allowed HTML tags are : `strong`, `em`, `a`, `p`, `ul`, `li`, `code`, `pre` (**required**).


#### <code>image</code> nature

Display an image with optional title and description.

JSON structure :
```
{
  statement: {
    nature: 'image',
    content: {
      url: <url>,
      title: <title>,
      description: <description>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `<url>` the image URL (**required**).
* `<title>` a noteworthy title.
* `<description>` a short description.


#### <code>video</code> nature

Display a video with optional title and description.

JSON structure :
```
{
  statement: {
    nature: 'video',
    content: {
      params: <params>,
      title: <title>,
      description: <description>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `<params>` the Youtube URL params; i.e. `https://www.youtube.com/embed/<params>` (**required**).
* `<title>` a noteworthy title.
* `<description>` a short description.


#### <code>map</code> nature

Display a map (via Google Maps Embed API) with optional title and description.

JSON structure :
```
{
  statement: {
    nature: 'map',
    content: {
      params: <params>,
      title: <title>,
      description: <description>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `<params>` the Google Maps params (**required**).
* `<title>` a noteworthy title for the whole map.
* `<description>` a short description for the whole map.

* In `<params>`, basically two kind of maps are available.
The first one embed a simple Google Maps Embed API URL like `https://www.google.com/maps/embed/v1/<params>` as follow :
```
    {
        api_key: <api_key>,
        endpoint: <endpoint>,
        query: <query_strings>
    }
```
The second (ie: `endpoint: "javascript"`) embed a subset of [Google Maps JavaScript API](https://developers.google.com/maps/documentation/javascript/reference/3.exp/) with optional markers :
```
    {
        api_key: <api_key>,
        endpoint: "javascript",
        payload: {
            map: <google.maps.MapOptions>,
            markers: {
              center: <true|false>,
              list: [{
                position: <google.maps.LatLngLiteral>,
                title: <marker_title>,
                description: <html_info_description>
              }]
            }
        }
    }
```


#### <code>button</code> nature

JSON structure :
```
{
  statement: {
    nature: 'button',
    content: {
      text: <text>,
      payload: <payload> | href: <url>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `<text>` is the displayed in the button (**required**).
* `<payload>` must be a Hash which can contain anything. It will be returned as is to the bot when the user click on the button (**required if no href**).
* `<href>` is a custom URL (**required if no payload**).

**Note:** `payload` and `href` are mutually exclusive.

#### <code>button_group</code> nature

JSON structure :
```
{
  statement: {
    nature: 'button_group',
    content: {
      disable_on_click: <boolean>,
      buttons: [
        {
          text: <text>,
          payload: <payload>
        },
        {
          text: <text>,
          payload: <payload>
        }
      ],
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `buttons` Array of buttons, from 1 to 6 buttons (**required**).
* `disable_on_click` boolean, default is `false`.

#### <code>card</code> nature

JSON structure :
```
{
  statement: {
    nature: 'card',
    content: {
      components: [
        {
          nature: <nature>,
          content:
          {
            <...>
          }
        },
      ],
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `components` Array of nested components (up to 5 components). Valid natures are `text`, `image`, `video`, `map`, `button`, `button_group`. Unlike standalone widgets, `speech` parameter on those components is ignored (**required**).


#### <code>list</code> nature

JSON structure :
```
{
  statement: {
    nature: 'list',
    content: {
      orientation: <orientation>,
      items: <components>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `orientation`: `vertical` or `horizontal`.
* `items` Array of nested components (from 2 to 8 components). Valid natures are `text`, `image`, `video`, `map`, `button`, `button_group`, `card`. Unlike standalone widgets, `speech` parameter on those components is ignored (**required**).


#### <code>geolocation</code> nature

JSON structure :
```
{
  statement: {
    nature: 'geolocation',
    content: {
      text: <text>,
      speech: {
        text: <speech_text>,
        locale: <speech_locale>
      }
    }
  }
}
```

* `text` : is the text displayed in the widget (**required**).

### Update chat session locale

Chat session locale is used for the speech to text functionality. User can choose it's locale, but API allow bot to dynamically change that preference.

#### Endpoint

`PUT /api/v1/chat_sessions/<session_id>`

#### JSON

```
{
  "chat_session": {
    "locale": "fr-FR"
  }
}
```

Available locales are the same as speech locales.


## Bot API

A bot is a remote service implementing the desired business logic.
It is reachable through a REST API and it **must include those following endpoints** in order to work with the viky.ai chatbot :

### Bot is alive

#### Endpoint

`GET /ping`

viky.ai wants to know if the bot is up and running mainly to display a status indicator for the user.

It expects an HTTP code `200 OK` if the bot is available. Any other code (or the lack of answer) will result in considering the bot is unavailable.

### Listen for new chat session

#### Endpoint

`POST /start`

#### JSON
```
{
  session_id: <id>
}
```
When a user starts a conversion from viky.ai, the bot will receive a new session id.

The bot must keep this session id if it wants to follow the context of several conversations at the same time.
For a specific bot, **only the last session is valid** and any request to a closed one will result in a `403 Forbidden` response.

### Listen for a new user action

#### Endpoint

`POST /sessions/<session_id>/user_actions`

#### JSON

```
{
  user_action: {
    type: <type>,
    ...
  }
}
```

A user made a new statement in the conversation. It must include the current session id as URL parameter.


### Types

User actions types triggered by the user.

#### Type <code>says</code>

```
{
  user_action: {
    type: 'says',
    text: <text>
  }
}
```

  * `text` the string typed by the user

#### Type <code>click</code>

```
{
  user_action: {
    type: 'click',
    payload: {
      <...>
    }
  }
}
```

  * `payload` the JSON hash previously passed at the button creation


#### Type <code>locate</code>

```
{
  user_action: {
    type: 'locate',
    status: 'success' | 'error',
    location: {<...>} | error: {<...>}
  }
}
```

  * `status` was the geolocation successful. In case of `success` there is a `location` key, otherwise an `error` value imply an `error` key.
  * `location` a JSON object representing the user position, see the [Geolocation API - Position](https://developer.mozilla.org/en-US/docs/Web/API/Position) **imply `status : 'success'`**
  * `error` a JSON object to explain why it was impossible to locate the user, see the [Geolocation API - PositionError](https://developer.mozilla.org/en-US/docs/Web/API/PositionError) **imply `status : 'error'`**
