require 'test_helper'

module Valgrind

  class ValgrindCommon < Nls::Common

    def setup

      resetDir

      # copy import file
      cp_import_fixture("several_packages_several_intents.json")

    end

    def interpret_queries

      interpret_simple_query=
      {
        "packages" => ["voqal.ai:datetime1"],
        "sentence" => "Hello Jean Marie",
        "Accept-Language" => "fr-FR"
      }

      # launch simple query
      700.times do
        response = Nls::Nls.interpret(interpret_simple_query)
        assert !response.nil?
      end

      hello_world_query = {
        timeout: 5,
        wait: "2000"
      }

      # launch hello world query with timeout
      expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"
      700.times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.query_get(Nls::Nls.url_test, hello_world_query)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

    end


  end
end
