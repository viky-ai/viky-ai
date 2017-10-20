require 'test_helper'

module Valgrind

  class ValgrindCommon < Nls::Common

    def setup

      resetDir


      @json_structure = Nls::Nls.several_packages_several_intents

      @main_uuid = "123456789"
      @main_package = Nls::Nls.create_package(@main_uuid , "voqal.ai:datetime1")
      @main_package["interpretations"] << Nls::Nls.create_interpretation("hello1")
      @main_package["interpretations"][0]["expressions"] << Nls::Nls.create_expression("Hello Brice")
      @main_package["interpretations"][0]["expressions"] << Nls::Nls.create_expression("Hello Jean Marie")

      @json_structure << @main_package

      generate_multiple_package_file(@json_structure)

    end

    def interpret_queries(nb_request_factor)

      # dump packages
      (30 * nb_request_factor).times do |i|
        actual_dump_result = Nls::Nls.query_get(Nls::Nls.url_dump)
        assert !actual_dump_result.nil?, "dump #{i}"
      end

      # launch simple query
      interpret_simple_query=
      {
        "packages" => [ @main_uuid ],
        "sentence" => "Hello Jean Marie",
        "Accept-Language" => "fr-FR"
      }

      (100 * nb_request_factor).times do
        response = Nls::Nls.interpret(interpret_simple_query)
        assert !response.nil?
      end

      # package update
      url_add = Nls::Nls.url_packages + "/#{@main_uuid}"

      expected_update_result =
      {
        "status" => "Package '#{@main_uuid}' successfully updated"
      }

      (30 * nb_request_factor).times do |i|
        actual_update_result = Nls::Nls.query_post(url_add, @main_package)
        assert_json expected_update_result, actual_update_result, "updating #{i}"
      end

      # launch hello world query with timeout
      hello_world_query = {
        timeout: 20,
        timeout_in: "NlpPackageGet"
      }

      expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"
      (30 * nb_request_factor).times do |i|
        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls::Nls.interpret(interpret_simple_query, hello_world_query)
        end
        assert_response_has_error expected_error, exception, "Timeout #{i}"
      end

    end


  end
end
