# Agents

## Interpret

```shell
curl -G "https://www.viky.ai/api/v1/agents/superman/superbot/interpret.json?" \
    --data-urlencode "now=2017-12-05T08:30:20+01:00" \
    --data-urlencode "sentence=I want to go from NY to LA" \
    --data-urlencode "spellchecking=high"
    --data-urlencode "context['session_id']='0112f97b-e5dd-47a4-b848'" \
    --data-urlencode "context['bot_version']='1.2.1'" \
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
      <td>spellchecking</td>
      <td>no</td>
      <td>high</td>
      <td>How wide the spellchecking is run. Possible values are : <code>inactive</code>, <code>low</code>, <code>medium</code> or <code>high</code>. </td>
    </tr>
    <tr>
      <td>now</td>
      <td>no</td>
      <td>2017-12-20T08:30:20+01:00</td>
      <td>The user current time with the UTC offset</td>
    </tr>
    <tr>
      <td>context</td>
      <td>no</td>
      <td>{ session_id: '0112f97b-e5dd-47a4-b848', bot_version: '1.2.1' }</td>
      <td>A custom hash for statsitics purpose.</td>
    </tr>
  </tbody>
</table>
