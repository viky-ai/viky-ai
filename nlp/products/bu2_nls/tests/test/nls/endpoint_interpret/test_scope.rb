# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestScope < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        @package_1 = create_package(1)
        @package_2 = create_package(2)

        Nls.package_update(@package_1)
        Nls.package_update(@package_2)
      end

      def create_package(id)
        package = Package.new("scope_package_#{id}")

        interpretation_public = package.new_interpretation("scope_public_#{id}", { scope: "public" })
        interpretation_public << Expression.new("hello scope")

        interpretation_private = package.new_interpretation("scope_private_#{id}", { scope: "private" })
        interpretation_private << Expression.new("hello scope")

        interpretation_hidden = package.new_interpretation("scope_hidden_#{id}", { scope: "hidden" })
        interpretation_hidden << Expression.new("hello scope")

        package
      end

      def test_scope
        check_interpret("hello scope", interpretations: ["scope_public_1"], packages: @package_1)
      end

      def test_scope_with_primary_and_show_private

        check_interpret("hello scope",
          interpretations: ["scope_public_1", "scope_public_2"],
          packages: [ @package_1, @package_2 ]
        )

        check_interpret("hello scope",
          interpretations: ["scope_public_1", "scope_public_2"],
          packages: [ @package_1, @package_2 ],
          show_private: true
        )

        check_interpret("hello scope",
          interpretations: ["scope_public_1"],
          packages: [ @package_1, @package_2 ],
          primary_package: @package_1
        )

        check_interpret("hello scope",
          interpretations: ["scope_public_2"],
          packages: [ @package_1, @package_2 ],
          primary_package: @package_2
        )

        interpretations = check_interpret("hello scope",
          interpretations: ["scope_public_1", "scope_private_1"],
          packages: [ @package_1, @package_2 ],
          show_private: true,
          primary_package: @package_1
        )
        assert_equal 'public',  interpretations['interpretations'][0]['scope']
        assert_equal 'private', interpretations['interpretations'][1]['scope']
        assert_equal 1.0, interpretations['interpretations'][0]['score']
        assert_equal 1.0, interpretations['interpretations'][1]['score']

        interpretations = check_interpret("hello scope",
          interpretations: ["scope_public_2", "scope_private_2"],
          packages: [ @package_1, @package_2 ],
          show_private: true,
          primary_package: @package_2
        )
        assert_equal 'public',  interpretations['interpretations'][0]['scope']
        assert_equal 'private', interpretations['interpretations'][1]['scope']
        assert_equal 1.0, interpretations['interpretations'][0]['score']
        assert_equal 1.0, interpretations['interpretations'][1]['score']

      end

    end
  end

end
