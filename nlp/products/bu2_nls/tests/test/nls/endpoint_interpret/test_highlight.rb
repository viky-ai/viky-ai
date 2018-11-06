# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestHighlight < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_parent_package)
      end

      def create_parent_package
        package = Package.new("parent_package")

        cheval = package.new_interpretation("cheval", { scope: "private" })
        cheval << Expression.new("cheval")

        blanc = package.new_interpretation("blanc", { scope: "private" })
        blanc << Expression.new("blanc")

        cheval_blanc = package.new_interpretation("cheval_blanc", { scope: "public" })
        cheval_blanc << Expression.new("@{cheval} @{blanc}", aliases: { "cheval" => cheval,  "blanc" => blanc })

        package
      end

      def create_child_package
        package = Package.new("child_package")



        package
      end

      def test_highlight
        expected_explanation = {
          scores: "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00",
          highlight: {
          summary: "[cheval] [blanc]",
            words: [
              {
                word: "cheval",
                match: {
                  expression: "cheval",
                  interpretation_slug: "cheval",
                  parent: {
                    expression: "cheval blanc",
                    interpretation_slug: "cheval_blanc",
                  }
                }
              },
              {
                word: "blanc",
                match: {
                  expression: "blanc",
                  interpretation_slug: "blanc",
                  parent: {
                    expression: "cheval blanc",
                    interpretation_slug: "cheval_blanc",
                  }
                }
              }
            ]
          },
          expression: {
            text: "@{cheval} @{blanc}",
            slug: "cheval_blanc",
            highlight: "[cheval] [blanc]",
            scores: "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00",
            expressions: [
              {
                text: "cheval",
                slug: "cheval",
                highlight: "[cheval] blanc",
                scores: "cov:0.50 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00",
                expressions: [
                  {
                    word: "cheval"
                  }
                ]
              },
              {
                text: "blanc",
                slug: "blanc",
                highlight: "cheval [blanc]",
                scores: "cov:0.50 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00",
                expressions: [
                  {
                    word: "blanc"
                  }
                ]
              }
            ]
          }
        }
        check_interpret("cheval blanc", interpretations: ["cheval_blanc"], explain: true, explanation: expected_explanation)
      end

    end
  end

end
