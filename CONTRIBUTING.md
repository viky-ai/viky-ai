# Contributing to viky.ai

This guide covers ways in which you can become a part of the ongoing development of viky.ai.

As mentioned in [README](README.md), everyone interacting in viky.ai codebases, issue trackers, chat rooms, and mailing lists is expected to follow this [code of conduct](CODE_OF_CONDUCT.md).


## Reporting an Issue

viky.ai uses [GitHub Issue Tracking](https://github.com/viky-ai/viky-ai/issues) to track issues. If you've found a bug, this is the place to start. You'll need to create a GitHub account in order to submit an issue, to comment on them, or to create pull requests.

If you've found a problem in viky.ai do a search on GitHub under [Issues](https://github.com/viky-ai/viky-ai/issues) in case it has already been reported. If you are unable to find any open issues addressing the problem you found, your next step will be to [open a new one](https://github.com/viky-ai/viky-ai/issues/new).

### Reporting bugs

Well-written bug reports are incredibly helpful. For that reason, your issue report should contain a title and a clear description of the issue. You should include as much relevant information as possible, including a clear description of the observed behaviour, a set of instructions for replicating it and the expected behaviour. Your objective is to facilitate the reproduction of the bug, to you and others, in order to find a solution.

### Reporting user interface bugs and features

If your bug or feature request touches on user interface, there are a few additional guidelines to follow:

* Include screenshots which are the visual equivalent of a minimal testcase.
* Make sure to include URLs, code snippets, and step-by-step instructions on how to reproduce the behavior visible in the screenshots.
* Indicate the operating system and web browser used.

### Requesting features

We're always trying to make viky.ai better, and your feature requests are a key part of that. Here are some tips on how to make a request:

* Please put your feature request items into GitHub Issues with the label "Feature request".
* Describe clearly and concisely what the missing feature is and how you'd like to see it implemented. Include example code (non-functional is OK) and/or UI mockups if possible.
* If it isn't obvious, explain why you'd like the feature.

As with most open-source projects, code talks. If you are willing to write the code for the feature yourself it's much more likely to be accepted. Fork viky.ai on GitHub, create a feature branch, and show us your work with a merge request!

### Helping to resolve existing issues

As a next step beyond reporting issues, you can help us to resolve existing ones by providing feedback about them.

* Can you reproduce the reported issue on your own computer? If so, you can add a comment to the issue saying that you're seeing the same thing.
* If an issue is vague, can you help us reduce it to something more specific? Perhaps you can provide additional information to help reproduce a bug.


## Contributing to the viky.ai documentation

You can help improve the viky.ai documentation by making it more coherent, consistent, or readable, adding missing information, correcting factual errors, fixing typos.

To do so, make changes to [viky.ai documentation source files](https://github.com/viky-ai/doc). Then open a pull request to apply your changes to the development branch.


## Contributing to the viky.ai webapp code

We will guide you through the contribution of a patch to viky.ai for the first time. The process is as follows:

1. Getting a copy of viky.ai code.
2. Running locally viky.ai platform.
3. Running locally viky.ai webapp tests suite.
4. Working on a feature
5. Submitting a pull request

### Getting a copy of viky.ai code

The first step to contributing to viky.ai is to get a copy of the source code. First, [fork viky.ai on GitHub](https://github.com/viky.ai/viky.ai/fork). Then, from the command line, navigate to the directory where you'll want your local copy of viky.ai to live and use the following command:

```bash
$ git clone https://github.com/YourGitHubName/viky-ai.git
```

### Install locally viky.ai platform

Install requirements describe in [README](README.md#requirements).

Setup the application using the following command:

```bash
$ cd webapp/
$ ./bin/setup
```

This setup script will perform the following tasks to run the webapp component in development mode:

* Installation of ruby gems (`bundle install`).
* Installation of JavaScript yarn packages (`yarn install`).
* Initialization of tables in the PostgreSQL database (`./bin/rails db:setup`).
* Initialization of the Elastic indexes (`./bin/rails statistics:setup`).
* Initialization of a `.env` file to define the minimum necessary environment variables.

Once the execution is finished, you can start the platform in development mode with the following command:

```bash
$ foreman start
```

The `foreman` command allows you to launch several commands defined in the `Procfile` file simultaneously. We use `docker-compose` to launch PostgresSQL, Redis, Elastic, Kibana and NLP services. Other commands runs Sidekiq to execute tasks asynchronously in the webapp and Webpack to bundle static assets . Finally, the Rails application is also started.

The application is now available at the following address: http://localhost:3000/

### Running locally viky.ai webapp tests suite

When you contribute to the viky.ai webapp component, it is very important that your code changes do not introduce bugs in other parts of the application. One way to check that the viky.ai webapp component still works after making your changes is to run its tests suite. If you have never run the webapp tests suite before, it is a good idea to run it once before to familiarize yourself with its result.

The webapp component being a Ruby On Rails application, you must execute the following commands to run the tests:

```bash
$ ./bin/rails test
$ ./bin/rails test:system
```

The second command starts the system tests which simulate how a real user would interact with the app. Don't worry and take a sit, it takes a few minutes.

### Working on a feature

We'll work on a "fake feature" as a case study:

<dl>
  <dt>Title</dt>
  <dd>Add hello method to <code>User</code> model</dd>
  <dt>Description</dt>
  <dd><code>User</code> model should provide an instance method called <code>hello</code> that returns "Hi".</dd>
</dl>

We'll now implement this feature and associated tests.

#### Creating a branch for the feature

Before making any changes, create a new branch for this ticket:

```bash
$ git checkout -b features/add_user_hello_method
```

At this step, you must choose a branch name that allows you to easily identify the ticket associated with the branch. All changes made in this branch must be specific to the ticket in progress.

#### Writing a test for the feature

In most cases, for a patch to be accepted into viky.ai it has to include tests. For bug fix patches, this means writing a regression test to ensure that the bug is never reintroduced into viky.ai later on.

A regression test should be written in such a way that it will fail while the bug still exists and pass once the bug has been fixed.

For patches containing new features, you'll need to include tests which ensure that the new features are working correctly. They should fail too when the new feature is not present, and then pass once it has been implemented.

In our case, we must write a model test to validate the implementation of the `hello` method. Navigate to `tests/models/` folder and edit file `user_test.rb`. Add the following code:

```ruby
test "hello method" do
  assert_equal "Hi", User.first.hello
end
```

Since we haven't made any modifications to `User` model yet, our test should fail. Let's run all the tests to make sure that's really what happens.

```bash
$ ./bin/rails test
```

If the tests ran correctly, you should see one failure corresponding to the test method we added, with this error:

```bash
Error:
UserTest#test_hello_method:
NoMethodError: undefined method `hello' for #<User:0x00007f9adcfec198>
    test/models/user_test.rb:6:in `block in <class:UserTest>'
```

#### Writing the code for your patch

We will now add the method `hello` to the `User` model.

Navigate to `app/models/` folder and edit file `user.rb`. Add the following code:

```ruby
def hello
  "Hi"
end
```

Now we need to make sure that the test we wrote earlier passes, so we can see whether the code we added is working correctly.

```bash
$ ./bin/rails test
```

Everything should pass. You can proceed to the next step.

### Submitting a pull request

#### Previewing your changes

Now it's time to go through all the changes made in our patch. To stage the changes ready for commit, run:

```bash
$ git add app/models/user.rb test/models/user_test.rb
```

Then display the differences between your current copy of viky.ai (with your changes) and the revision that you initially checked out earlier with:

```bash
$ git diff --cached
```

Use the arrow keys to move up and down. Hit the **q** key to return to the command line.

```diff
diff --git a/webapp/app/models/user.rb b/webapp/app/models/user.rb
index ec3adfdc..a5f9d0be 100644
--- a/webapp/app/models/user.rb
+++ b/webapp/app/models/user.rb
@@ -26,6 +26,10 @@ class User < ApplicationRecord
   before_validation :clean_username
   before_destroy :check_agents_presence, prepend: true

+  def hello
+    "Hi"
+  end
+
   def can?(action, agent)
     return false unless [:edit, :show].include? action
     return true  if action == :show && agent.is_public?
diff --git a/webapp/test/models/user_test.rb b/webapp/test/models/user_test.rb
index 93c789c8..8c2c426a 100644
--- a/webapp/test/models/user_test.rb
+++ b/webapp/test/models/user_test.rb
@@ -2,6 +2,10 @@ require 'test_helper'

 class UserTest < ActiveSupport::TestCase

+  test "hello method" do
+    assert_equal "Hi", User.first.hello
+  end
+
   test "non admin user" do
     assert_not User.find_by_email('notconfirmed@viky.ai').admin?
     assert User.find_by_email('admin@viky.ai').admin?
```


If the patch's content looked okay, it's time to commit the changes.

```bash
$ git commit . -m "User instance can say hello"
```

#### Pushing the commit and making a pull request

After committing the patch, send it to your fork on GitHub.

```bash
$ git push origin features/add_user_hello_method
```

You can now create a pull request by visiting the [viky.ai GitHub page](https://github.com/viky-ai/viky-ai/). You'll see your branch under "Your recently pushed branches". Click "Compare & pull request" next to it and create the pull request.
