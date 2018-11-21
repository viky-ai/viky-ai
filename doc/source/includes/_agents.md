# Agents

## Interpret

```shell
curl -G "https://www.viky.ai/api/v1/agents/superman/superbot/interpret.json?now=2017-12-05T08:30:20+01:00" --data-urlencode "sentence=I want to go from NY to LA" \
  -H "Agent-Token: the-agent-token" \
  -H "Accept-Language: en-gb"
```

> JSON response:

```json
{
  "interpretations": [
    {
      "id": "db3dd17d-fd3f-42a7-9d65-fd5a09894015",
      "slug": "superman/superbot/trip",
      "name": "trip",
      "score": 0.97,
      "solution": {
        "from": "NY",
        "to": "LA"
      }
    }
  ]
}
```

This endpoint retrieves all the interpretations extracted from the `sentence` string passed as parameter.

### HTTP Request

`GET /agents/:username/:agentname/interpret.json`

### Query Parameters


<table>
  <thead>
    <tr>
      <th>Parameter</th>
      <th>Mandatory</th>
      <th>Example</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>:username</td>
      <td>yes</td>
      <td>superman</td>
      <td>The username of the agent's owner.</td>
    </tr>
    <tr>
      <td>:agentname</td>
      <td>yes</td>
      <td>superbot</td>
      <td>The name of the agent.</td>
    </tr>
    <tr>
      <td>sentence</td>
      <td>yes</td>
      <td>I'm looking for a train to Paris</td>
      <td>The sentence to be processed by the AI.</td>
    </tr>
    <tr>
      <td>language</td>
      <td>no</td>
      <td>en-US, en;q=0.9</td>
      <td>The language code in which the sentence is written.</td>
    </tr>
    <tr>
      <td>now</td>
      <td>no</td>
      <td>2017-12-20T08:30:20+01:00</td>
      <td>The user current time with the UTC offset</td>
    </tr>
  </tbody>
</table>

<aside class="notice">
  Remember — the language can be specified also in the request <code>Accept-Language</code> header using all country variants and the q-weighting factors, e.g.: <code>Accept-Language: fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5</code>
</aside>

### Headers

Besides the <code>Agent-Token</code> and <code>Accept-Language</code> headers, headers beginning with <code>Context</code> can be used to provide additional metadata about the request.

<table class="header-table">
  <thead>
    <tr>
      <th>Header</th>
      <th>Mandatory</th>
      <th>Example</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Agent-Token</td>
      <td>yes</td>
      <td>22d40714c5dbde3eb86a179a0729f486</td>
      <td>The API token of the agent.</td>
    </tr>
    <tr>
      <td>Accept-Language</td>
      <td>no</td>
      <td>fr-CH, fr;q=0.9</td>
      <td>The language code in which the sentence is written.</td>
    </tr>
    <tr>
      <td>Context-Client-Type</td>
      <td>no</td>
      <td>console</td>
      <td>The medium through which the API is accessed.</td>
    </tr>
    <tr>
      <td>Context-User-Id</td>
      <td>no</td>
      <td>78945698</td>
      <td>The unique ID of the user.</td>
    </tr>
    <tr>
      <td>Context-Session-Id</td>
      <td>no</td>
      <td>0112f97b-e5dd-47a4-b848</td>
      <td>The ID of the session in case of chat sessions.</td>
    </tr>
    <tr>
      <td>Context-Bot-Version</td>
      <td>no</td>
      <td>1.2.1</td>
      <td>The bot version if the client type is 'bot'.</td>
    </tr>
    <tr>
      <td>Context-Test</td>
      <td>no</td>
      <td>viky-test-agents</td>
      <td>The type of test performed if the request was made for test purposes.</td>
    </tr>
  </tbody>
</table>

