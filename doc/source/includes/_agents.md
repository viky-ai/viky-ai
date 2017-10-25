# Agents

## Interpret

```shell
curl "https://www.viky.ai/api/v1/agents/superman/superbot/interpret?sentence=hello"
  -H "Agent-Token: the-agent-token"
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

`GET /agents/:user_id/:id/interpret`

### Query Parameters

Parameter | Mandatory | Default | Example | Description
--------- | --------- | ------- | ------- | -----------
:user_id | yes |  | superman | The username of the agent's owner.
:id | yes |  | superbot | The name of the agent.
sentence | yes |  | I'm looking for a train to Paris | The sentence to be processed by the AI.
language | no | en-US | en-US, en;q=0.9 | The language code in which the sentence is written.

<aside class="success">
Remember — the language can be specified also in the request <code>Accept-Language</code> header using all country variants and the q-weighting factors, e.g.:<br/>
<code>Accept-Language: fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5</code>
</aside>
