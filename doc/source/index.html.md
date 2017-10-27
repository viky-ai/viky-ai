---
title: API Reference

language_tabs: # must be one of https://git.io/vQNgJ
  - shell

includes:
  - agents

search: true
---

# Introduction

Welcome to the viky.ai API! You can use our API to access bot building and AI endpoints.
We have language bindings in Shell and more to come in Ruby, Javascript and other languages!

# Authentication

```shell
# With shell, you can just pass the correct resource token header with each request
curl "https://www.viky.ai/api/v1/superman/superbot/:action" \
  -H "Agent-Token: the-superbot-api-token"

# or pass the token as a parameter in the URL
curl "https://www.viky.ai/api/v1/superman/superbot/:action?agent_token=the-superbot-api-token"
```

> Make sure to replace `the-superbot-api-token` with the actual resource API token.

To access viky.ai resources, e.g. the `agent` endpoint for interpretation, you must specify the resource specific api token in the request header:

`Agent-Token: the-superbot-api-token`

or as a URL parameter:

`?agent_token=the-superbot-api-token`

<aside class="notice">
  You must replace <code>the-superbot-api-token</code> with your personal API key.
</aside>

# Errors

<aside class="notice">
  Error responses always have a message associated with in the <code>message</code> field
</aside>

The viky.ai API uses the following error codes:

<table>
  <thead>
    <tr>
      <th>Error&nbsp;Code</th>
      <th>Meaning</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>401</code></td>
      <td>Unauthorized: Agent API token is wrong.</td>
    </tr>
    <tr>
      <td><code>404</code></td>
      <td>Not Found: The specified agent could not be found.</td>
    </tr>
    <tr>
      <td><code>406</code></td>
      <td>Not Acceptable: You requested a format that isn't json.</td>
    </tr>
    <tr>
      <td><code>422</code></td>
      <td>Unprocessable Entity: Parameters are not consistent.</td>
    </tr>
    <tr>
      <td><code>500</code></td>
      <td>Internal Server Error: We had a problem with our server. Try again later.</td>
    </tr>
  </tbody>
</table>
