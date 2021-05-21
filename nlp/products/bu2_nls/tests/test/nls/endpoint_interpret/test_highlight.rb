# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestHighlight < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

        Nls.package_update(create_parent_package)
        Nls.package_update(create_any_package)
        Nls.package_update(create_number_package)
        Nls.package_update(create_regex_package)
      end

      def create_parent_package
        package = Package.new("parent_package")

        cheval = package.new_interpretation("cheval", { scope: "private" })
        cheval << Expression.new("cheval")

        blanc = package.new_interpretation("blanc", { scope: "private" })
        blanc << Expression.new("blanc")

        cheval_blanc = package.new_interpretation("cheval_blanc", { scope: "public" })
        cheval_blanc << Expression.new("@{cheval} @{blanc}", aliases: { "cheval" => cheval, "blanc" => blanc })

        package
      end

      def create_any_package
        package = Package.new("any_package")

        before = package.new_interpretation("before", { scope: "private" })
        before << Expression.new("before")

        after = package.new_interpretation("after", { scope: "private" })
        after << Expression.new("after")

        with_any = package.new_interpretation("with_any", { scope: "public" })
        with_any << Expression.new("@{before} @{any} @{after}", aliases: { "before" => before, "any" => Alias.any, "after" => after }, keep_order: true)

        package
      end

      def create_number_package
        package = Package.new("number_package")

        numbers = package.new_interpretation("numbers", { scope: "public" })
        numbers << Expression.new("testNumber @{number}", aliases: { "number" => Alias.number }, keep_order: true)

        package
      end

      def create_regex_package
        package = Package.new("package")

        email = package.new_interpretation("email", { scope: "private" })
        email << Expression.new("@{email}", aliases: { email: Alias.regex("[a-z]+@[a-z]+\.[a-z]+") }, solution: "`{ email: email }`")

        sentence = package.new_interpretation("sentence", { scope: "public" })
        sentence << Expression.new("email: @{email}", aliases: { email: email}, keep_order: true)

        package
      end

      def test_highlight_basic
        expected_explanation = {
          scores: "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00 expr:0.00 ord:1.00",
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
                    interpretation_slug: "cheval_blanc"
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
                    interpretation_slug: "cheval_blanc"
                  }
                }
              }
            ]
          },
          expression: {
            text: "@{cheval} @{blanc}",
            slug: "cheval_blanc",
            highlight: "[cheval] [blanc]",
            scores: "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00 expr:0.00 ord:1.00",
            expressions: [
              {
                text: "cheval",
                slug: "cheval",
                highlight: "[cheval] blanc",
                scores: "cov:0.50 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00 expr:0.00 ord:1.00",
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
                scores: "cov:0.50 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:1.00 expr:0.00 ord:1.00",
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

      def test_highlight_wording
        expected_explanation = {
          highlight: {
          summary: "[chevale] [blanc]",
            words: [
              {
                word: "chevale",
                match: {
                  expression: "chevale",
                  interpretation_slug: "cheval",
                  parent: {
                    expression: "chevale blanc",
                    interpretation_slug: "cheval_blanc"
                  }
                }
              },
              {
                word: "blanc",
                match: {
                  expression: "blanc",
                  interpretation_slug: "blanc",
                  parent: {
                    expression: "chevale blanc",
                    interpretation_slug: "cheval_blanc"
                  }
                }
              }
            ]
          },
          expression: {
            text: "@{cheval} @{blanc}",
            slug: "cheval_blanc",
            highlight: "[chevale] [blanc]",
            expressions: [
              {
                text: "cheval",
                slug: "cheval",
                highlight: "[chevale] blanc",
                expressions: [
                  {
                    word: "cheval"
                  }
                ]
              },
              {
                text: "blanc",
                slug: "blanc",
                highlight: "chevale [blanc]",
                expressions: [
                  {
                    word: "blanc"
                  }
                ]
              }
            ]
          }
        }
        check_interpret("chevale blanc", interpretations: ["cheval_blanc"], spellchecking: :medium, explain: true, explanation: expected_explanation)
      end

      def test_highlight_number_english_format
        expected_explanation = {
          highlight: {
            summary: "[testNumber] [12,345.678]",
            words: [
              {
                word: "testNumber",
                match: {
                  expression: "testNumber 12,345.678",
                  interpretation_slug: "numbers"
                }
              },
              {
                word: "12,345.678",
                match: {
                  expression: "testNumber 12,345.678",
                  interpretation_slug: "numbers"
                }
              }
            ]
          },
          expression: {
            text: "testNumber @{number}",
            slug: "numbers",
            highlight: "[testNumber] [12,345.678]",
            expressions: [
              {
                word: "testnumber"
              },
              {
                word: "12345.678"
              }
            ],
          }
        }
        check_interpret("testNumber 12,345.678", interpretations: ["numbers"], explain: true, explanation: expected_explanation)
      end

      def test_highlight_number_french_format
        expected_explanation = {
          highlight: {
            summary: "[testNumber] [12 345,678]",
            words: [
              {
                word: "testNumber",
                match: {
                  expression: "testNumber 12 345,678",
                  interpretation_slug: "numbers",
                }
              },
              {
                word: "12 345,678",
                match: {
                  expression: "testNumber 12 345,678",
                  interpretation_slug: "numbers",
                }
              }
            ]
          },
          expression: {
            text: "testNumber @{number}",
            slug: "numbers",
            highlight: "[testNumber] [12 345,678]",
            expressions: [
              {
                word: "testnumber"
              },
              {
                word: "12345.678"
              }
            ],
          }
        }
        Interpretation.default_locale = "fr-FR"
        check_interpret("testNumber 12 345,678", interpretations: ["numbers"], explain: true, explanation: expected_explanation)
      end

      def test_highlight_any
        expected_explanation = {
          highlight: {
            summary: "[before] any [after]",
            words: [
              {
                word: "before",
                match: {
                  expression: "before",
                  interpretation_slug: "before",
                  parent: {
                    expression: "before any after",
                    interpretation_slug: "with_any"
                  }
                }
              },
              {
                is_any: true,
                word: "any",
                match: {
                  expression: "before any after",
                  interpretation_slug: "with_any"
                }
              },
              {
                word: "after",
                match: {
                  expression: "after",
                  interpretation_slug: "after",
                  parent: {
                    expression: "before any after",
                    interpretation_slug: "with_any"
                  }
                }
              }
            ]
          },
          expression: {
            text: "@{before} @{any} @{after}",
            slug: "with_any",
            highlight: "[before] any [after]",
            expressions: [
              {
                text: "before",
                slug: "before",
                highlight: "[before] any after",
                expressions: [
                  {
                    word: "before"
                  }
                ]
              },
              {
                text: "after",
                slug: "after",
                highlight: "before any [after]",
                expressions: [
                  {
                    word: "after"
                  }
                ]
              },
              {
                any: "any",
                highlight: "before [any] after"
              }
            ],
            computed_solution: {
              any: "any"
            }
          }
        }

        check_interpret("before any after", interpretations: ["with_any"], explain: true, explanation: expected_explanation)
      end

      def test_highlight_regex
        expected_explanation = {

          highlight: {
            summary: "[email][:] [sebastien@pertimm.com]",
            words: [
              {
                word: "email",
                match: {
                  expression: "email: sebastien@pertimm.com",
                  interpretation_slug: "sentence",
                }
              },
              {
                word: ":",
                match: {
                  expression: "email: sebastien@pertimm.com",
                  interpretation_slug: "sentence",
                }
              },
              {
                word: "sebastien@pertimm.com",
                match: {
                  expression: "sebastien@pertimm.com",
                  interpretation_slug: "email",
                  parent: {
                    expression: "email: sebastien@pertimm.com",
                    interpretation_slug: "sentence",
                  }
                }
              }
            ]
          },
          expression: {
            text: "email: @{email}",
            slug: "sentence",
            highlight: "[email][:] [sebastien@pertimm.com]",
            expressions: [
              {
                word: "email"
              },
              {
                word: ":"
              },
              {
                text: "@{email}",
                slug: "email",
                highlight: "email: [sebastien@pertimm.com]",
                expressions: [
                  {
                    word: "sebastien@pertimm.com"
                  }
                ],
                package_solution: "`{ email: email }`",
                computed_solution: {
                  email: "sebastien@pertimm.com"
                }
              }
            ],
            computed_solution: {
              email: "sebastien@pertimm.com"
            }
          }
        }

        check_interpret("email: sebastien@pertimm.com", interpretations: ["sentence"], explain: true, explanation: expected_explanation)
      end

    end
  end

end
