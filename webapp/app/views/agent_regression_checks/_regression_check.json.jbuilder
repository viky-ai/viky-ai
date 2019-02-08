json.id        test.id
json.state     test.state
json.sentence  test.sentence
json.language  test.language
json.expected  test.expected
json.got       test.got
json.timestamp Time.now.to_f * 1000
json.delete_url  user_agent_agent_regression_check_path(user, test.agent, test)
