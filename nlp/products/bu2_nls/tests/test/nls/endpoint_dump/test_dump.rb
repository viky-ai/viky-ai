# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestDump < NlsTestCommon

      def test_nls_dump
        Nls.remove_all_packages

        package = Package.new("package")
        i_first_interpretation = Interpretation.new("interpretation_1", {solution: {"agaga" => "ceci est une solution"}})
                                                    .new_textual("mytextual", {
                                                                              keep_order: Expression.keep_order,
                                                                              glued: Expression.glued,
                                                                              solution: {"to" => "`town.any`"}
                                                                            })
        package << i_first_interpretation

        i_second_interpretation_params = {
                aliases: {'first': i_first_interpretation, any: Alias.any, digits: Alias.digit},
                keep_order: Expression.keep_order,
                solution: {"to" => "`town.name`"}
              }
        i_second_interpretation = Interpretation.new("interpretation_2", {solution: {"deuze" => "ceci est une autre solution",
                                                                                     "troize" => "et de 3! "}})
                                                    .new_expression("@expression_1", i_second_interpretation_params)
        package << i_second_interpretation

        Nls.package_update(package)
        actual = Nls.dump

        assert_equal package.to_h, actual[0]

      end

    end
  end
end
