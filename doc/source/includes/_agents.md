# Agents

## Interpret

```shell
curl "https://www.viky.ai/api/v1/agents/superman/superbot/interpret.json?sentence=hello" \
  -H "Agent-Token: the-agent-token" \
  -H "Accept-Language: en-gb"
```

> JSON response:

```json
{
  "intents": [
    {
      "id": "db3dd17d-fd3f-42a7-9d65-fd5a09894015",
      "slug": "superman/superbot/hello",
      "name": "hello",
      "score": 0.7
    },
    {
      "id": "affdd17d-fd3f-90a7-9d65-fd5a09893033",
      "slug": "superman/superbot/hello-world",
      "name": "hello-world",
      "score": 0.5
    }
  ]
}
```

This endpoint retrieves all the intents extracted from the `sentence` string passed as parameter.

### HTTP Request

`GET /agents/:username/:agentname/interpret`

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
      <td>The language code in which the sentence is written. Default value: <code>en-US</code></td>
    </tr>
  </tbody>
</table>

<aside class="notice">
  Remember — the language can be specified also in the request <code>Accept-Language</code> header using all country variants and the q-weighting factors, e.g.: <code>Accept-Language: fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5</code>
</aside>
