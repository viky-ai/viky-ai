---
title: API Reference

language_tabs: # must be one of https://git.io/vQNgJ
  - shell

toc_footers:
  - <a href='https://www.vicky.ai'>Viky.ai Home</a>

includes:
  - agents

search: true
---

# Introduction

Welcome to the Viky.ai API! You can use our API to access bot building and AI endpoints.
We have language bindings in Shell and more to come in Ruby, Javascript and other languages!

# Base URI

The API is versioned and the current base endpoint is:

`https://www.viky.ai/api/v1`

This will be referred as `<API_BASE_URI>` all along the documentation.

# Authentication

```shell
# With shell, you can just pass the correct resource token header with each request
curl "<API_BASE_URI>/superman/superbot/:action"
  -H "Agent-Token: the-superbot-api-token"

# or pass the token as a parameter in the URL
curl "<API_BASE_URI>/superman/superbot/:action?agent_token=the-superbot-api-token"
```

> Make sure to replace `the-superbot-api-token` with the actual resource API token.

To access Viky.ai resources, e.g. the `agent` endpoint for interpretation, you must specify the resource specific api token in the request header:

`Agent-Token: the-superbot-api-token`

or as a URL parameter:

`?agent_token=the-superbot-api-token`

<aside class="notice">
You must replace <code>the-superbot-api-token</code> with your personal API key.
</aside>

# Errors

<aside class="notice">Error responses always have a message associated with in the <code>message</code> field, and, in case of a severe 500 status error they also bring the stack trace in the <code>application_trace</code> field.</aside>

The Viky.ai API uses the following error codes:


Error Code | Meaning
---------- | -------
401 | Unauthorized -- Agent API token is wrong.
404 | Not Found -- The specified agent could not be found.
406 | Not Acceptable -- You requested a format that isn't json.
422 | Unprocessable Entity -- Parameters are not consistent.
500 | Internal Server Error -- We had a problem with our server. Try again later.
