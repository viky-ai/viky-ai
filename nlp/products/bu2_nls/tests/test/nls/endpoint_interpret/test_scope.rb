# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestScope < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("scope_package")

        interpretation_public = package.new_interpretation("scope_public", { scope: "public" })
        interpretation_public << Expression.new("hello scope")

        interpretation_private = package.new_interpretation("scope_private", { scope: "private" })
        interpretation_private << Expression.new("hello scope")

        interpretation_hidden = package.new_interpretation("scope_hidden", { scope: "hidden" })
        interpretation_hidden << Expression.new("hello scope")

        package
      end

      def test_scope
        check_interpret("hello scope", interpretations: ["scope_public", "scope_private"])
      end

      # TODO 16406 implement private scope
    end
  end

end
