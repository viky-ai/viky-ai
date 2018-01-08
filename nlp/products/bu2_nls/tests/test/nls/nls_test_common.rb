module Nls

  class NlsTestCommon < TestCommon

    attr_reader :available_packages

    @available_packages = {}
    @packages_dump = []

    def importDir
      File.join(File.expand_path(pwd), "import")
    end

    def resetDir

      @available_packages = {}
      @packages_dump = []

      FileUtils.rm_r(importDir, force: true)
      FileUtils.mkdir_p(importDir)
    end

    def setup

      resetDir

      Nls.start
    end

    Minitest.after_run do
      Nls.stop
    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

    def fixture_parse(file)
      JSON.parse(File.read(fixture_path(file)))
    end

    def cp_import_fixture(file)
      FileUtils.cp(fixture_path(file), importDir)
    end

    def json_interpret_body(package, sentence, locale = Interpretation.default_locale, explain = false, now=nil)
      request = {}

      if package == "*"
        request['packages'] = "*"
      elsif package.kind_of? Array
        package_ids = package.map do |p|
          if p.kind_of? Package
            p.id.to_s
          else
            p
          end
        end
        request['packages'] = package_ids
      else
        package_id = package
        package_id = package.id.to_s if package.kind_of? Package
        request['packages'] = [package_id]
      end

      request['sentence'] = sentence
      request['Accept-Language'] = locale if !locale.nil?
      request['now'] = now if !now.nil?
      request['show-explanation'] = true  if explain
      request
    end

    def expected_update_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully updated"
      }
    end

    def expected_delete_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully deleted"
      }
    end

    def expected_added_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully updated"
      }
    end

    def several_packages_several_intents

      Interpretation.default_locale = "fr-FR"

      datetime1 = Package.new('datetime1')
      datetime1_hello1 = Interpretation.new('hello1')
      datetime1_hello1 << Expression.new('Hello Brice',{locale: Interpretation.default_locale})
      datetime1_hello1 << Expression.new('Hello Jean Marie',{locale: Interpretation.default_locale})
      datetime1 << datetime1_hello1

      datetime1_hello2 = Interpretation.new('hello2')
      datetime1_hello2 << Expression.new('Hello Nicolas',{locale: Interpretation.default_locale})
      datetime1_hello2 << Expression.new('Hello Olivier',{locale: Interpretation.default_locale})
      datetime1 << datetime1_hello2

      datetime1_hello3 = Interpretation.new('hello3')
      datetime1_hello3 << Expression.new('Hello Sebastien',{locale: Interpretation.default_locale})
      datetime1_hello3 << Expression.new('Hello Enrico',{locale: Interpretation.default_locale})
      datetime1_hello3 << Expression.new('Hello Mouadh',{locale: Interpretation.default_locale})
      datetime1 << datetime1_hello3
      @available_packages[datetime1.slug] = datetime1
      @packages_dump << datetime1.to_h

      datetime2 = Package.new('datetime2')
      datetime2_hello1 = Interpretation.new('hello1')
      datetime2_hello1 << Expression.new('Hello Nicolas',{locale: Interpretation.default_locale})
      datetime2_hello1 << Expression.new('Hello Olivier',{locale: Interpretation.default_locale})
      datetime2 << datetime2_hello1
      @available_packages[datetime2.slug] = datetime2
      @packages_dump << datetime2.to_h

      datetime3 = Package.new('datetime3')
      datetime3_hello2 = Interpretation.new('hello2')
      datetime3_hello2 << Expression.new('Hello Sebastien',{locale: Interpretation.default_locale})
      datetime3_hello2 << Expression.new('Hello Enrico',{locale: Interpretation.default_locale})
      datetime3_hello2 << Expression.new('Hello Mouadh',{locale: Interpretation.default_locale})
      datetime3 << datetime3_hello2
      @available_packages[datetime3.slug] = datetime3
      @packages_dump << datetime3.to_h

      datetime4 = Package.new('samesentence1')
      datetime4_hello4 = Interpretation.new('hello4')
      datetime4_hello4 << Expression.new('Hello Sebastien',{locale: Interpretation.default_locale})
      datetime4_hello4 << Expression.new('Hello Enrico',{locale: Interpretation.default_locale})
      datetime4_hello4 << Expression.new('Hello Mouadh',{locale: Interpretation.default_locale})
      datetime4 << datetime4_hello4
      @available_packages[datetime4.slug] = datetime4
      @packages_dump << datetime4.to_h

      datetime5 = Package.new('samesentence2')
      datetime5_hello5 = Interpretation.new('hello5')
      datetime5_hello5 << Expression.new('Hello Sebastien',{locale: Interpretation.default_locale})
      datetime5_hello5 << Expression.new('Hello Enrico',{locale: Interpretation.default_locale})
      datetime5_hello5 << Expression.new('Hello Mouadh',{locale: Interpretation.default_locale})
      datetime5 << datetime5_hello5
      @available_packages[datetime5.slug] = datetime5
      @packages_dump << datetime5.to_h

      @available_packages.values.each do |package|
        package.to_file(importDir)
      end

      @available_packages.values

    end

    def create_building_feature_any
      pg_building_feature = Package.new("pg-building-feature")

      i_sea_view = Interpretation.new("sea-view").new_textual(["sea view", "sea front", "ocean view", "ocean front"], solution: "Sea view" )
      pg_building_feature << i_sea_view

      i_swimming_pool = Interpretation.new("swimming-pool").new_textual(["swimming pool", "pool"], solution: "swimming pool" )
      pg_building_feature << i_swimming_pool

      i_building_feature = Interpretation.new("building-feature")
      i_building_feature.new_expression("@{swimming-pool}", aliases: {'swimming-pool' => i_swimming_pool})
      i_building_feature.new_expression("@{sea-view}", aliases: {'sea-view'  => i_sea_view})
      pg_building_feature << i_building_feature

      i_preposition_building_feature= Interpretation.new("preposition-building-feature").new_textual(["with", "at"])
      pg_building_feature << i_preposition_building_feature

      pg_building_feature_hash = {'preposition_building_feature'  => i_preposition_building_feature, 'building_feature'  => i_building_feature}
      pg_building_feature_any_hash = {'preposition_building_feature'  => i_preposition_building_feature, 'building_feature'=> Alias.any}
      i_pg_building_feature = Interpretation.new("pg-building-feature")
      i_pg_building_feature.new_expression("@{preposition_building_feature} @{building_feature}",
          aliases: pg_building_feature_hash,
          keep_order: true,
          solution: { building_feature: "`building_feature`" })
      i_pg_building_feature.new_expression("@{preposition_building_feature} @{building_feature}",
          aliases: pg_building_feature_any_hash,
          keep_order: true,
          solution: { building_feature: "`building_feature`" })
      i_pg_building_feature.new_expression("@{building_feature}",
          aliases: {'building_feature'  => i_building_feature},
          solution: { building_feature: "`building_feature`" })
      pg_building_feature << i_pg_building_feature


      i_pg_building_features = Interpretation.new("pg-building-features")
      pg_building_features_hash = { features: i_pg_building_feature, features_recursive: i_pg_building_features}
      i_pg_building_features.new_expression("@{features} @{features_recursive}", aliases: pg_building_features_hash )
      i_pg_building_features.new_expression("@{features}", aliases: { features: i_pg_building_feature }, keep_order: true )
      pg_building_feature << i_pg_building_features
      @available_packages[pg_building_feature.slug] = pg_building_feature

      pg_building_feature
    end


    def full_minimal_package(package_slug, interpretation_slug, expression)
      package = Package.new(package_slug)
      interpretation = Interpretation.new(interpretation_slug)
      interpretation << Expression.new(expression, {locale: Interpretation.default_locale})
      package << interpretation
      package
    end

    def package_to_update(uuid, slug = "titi", interpretation = "toto", expression="Hello Brice")
      json_package = Package.new(slug, id: uuid)
      json_interpretation = Interpretation.new(interpretation)
      json_interpretation << Expression.new(expression, {locale: Interpretation.default_locale})
      json_package << json_interpretation
      json_package
    end

    def check_interpret(sentence, expected)

      raise "expected must be an Hash"   if !expected.kind_of? Hash
      raise "expected must not be empty" if expected.empty?

      debug = expected[:debug]

      now = expected[:now]

      ap expected if debug
      # creation et exécution de la requete
      request = json_interpret_body("*", sentence, Interpretation.default_locale, false, now)
      ap request if debug
      actual = Nls.interpret(request)
      ap actual if debug

      assert_kind_of Hash, actual, "Actual answer is not an Hash : #{actual.inspect}"
      assert_kind_of Array, actual['interpretations'], "Actual answer['interpretations'] is not an Array : #{actual['interpretations']}"

      if expected.has_key?(:interpretations) && expected[:interpretations].kind_of?(Array) && expected[:interpretations].empty?
         assert actual['interpretations'].empty?, "Actual answer should not match on any interpretation"
         # skip other assert
         return actual
      end

      assert !actual['interpretations'].empty?, "Actual answer did not match on any interpretation"

      match_intepretation = actual['interpretations'].first

      if expected.has_key?(:warnings)

        assert_kind_of Array, actual['warnings'], "Actual answer['warnings'] is not an Array : #{actual.inspect}"

        expected_warnings = expected[:warnings]

        expected_warning_found = false
        actual['warnings'].each do |warning|
          if warning.include?(expected_warnings)
            expected_warning_found = true
            break
          end
        end

        assert expected_warning_found, "Actual answer['warnings'] must contains \"#{expected_warnings}\": \n#{JSON.generate(actual['warnings'])}"

      else
        if !actual['warnings'].nil?
          assert false, "Actual answer must be without warnings :\n#{actual['warnings'].join("\n")}"
        end
      end


      if expected.has_key?(:interpretations)
        expected_interpretations = expected[:interpretations]

        match_intepretations_slug = []
        actual['interpretations'].each do |match_intepretation_local|
          match_intepretations_slug << match_intepretation_local['slug']
        end

        assert_equal expected_interpretations, match_intepretations_slug, "match on wrong interpretation"
      end

      if expected.has_key?(:interpretation)
        expected_interpretation = expected[:interpretation]
        slug_match = match_intepretation['slug'] == expected_interpretation || match_intepretation['id'] == expected_interpretation
        assert slug_match, "match on wrong interpretation : id = #{match_intepretation['id']}, slug = #{match_intepretation['slug']}"
      end

      if expected.has_key?(:solution)
        expected_solution = expected[:solution]
        if expected_solution.kind_of?(Hash) || expected_solution.kind_of?(Array)
          expected_solution.deep_stringify_keys!
        end

        if expected_solution.nil?
          assert_nil match_intepretation['solution'], "Matched on unexpected solution (nil)"
        else
          assert_equal expected_solution, match_intepretation['solution'], "Matched on unexpected solution"
        end
      end

      if expected.has_key?(:score)
        expected_score = expected[:score]
        assert_equal expected_score, match_intepretation['score'], "Matched on wrong score"
      end

      return actual

    end


    def assert_exception_has_message expected_error, exception, msg = nil

      header = ''
      header = "#{msg} : " if !msg.nil?

      if !expected_error.nil? && !expected_error.empty?
        assert !exception.nil?, "#{header}exception must not be nil : #{exception.inspect}"
        assert !exception.message.nil?, "#{header}exception message must not be nil : #{exception.inspect}"
        assert !exception.message.empty?, "#{header}exception message must not be empty : #{exception.inspect}"

        expected_error_found = false
        expected_error_found = true  if exception.message.include? expected_error

        assert expected_error_found, "#{header}exception message must contains \"#{expected_error}\": \ntoto#{exception.inspect}"

      end

    end

    def assert_json expected, actual, msg = nil

      header = ''
      header = "#{msg} : " if !msg.nil?

      expected_json = {}
      if expected.kind_of?(Array) || expected.kind_of?(Hash)
        if expected.kind_of?(Array)
          expected.map do |e|
            if e.kind_of?(Hash)
              e.deep_stringify_keys!
            end
          end
        elsif expected.kind_of?(Hash)
          expected.deep_stringify_keys!
        end
        expected_json = expected
      elsif expected.kind_of?(String)
        if File.exist?(expected)
          expected_json = JSON.parse(File.read(expected))
        else
          assert false, "#{header}expected file path must exists, expected: #{expected}"
        end
      else
        assert false, "#{header}expected must a file path or a json object, expected: #{expected.inspect}"
      end

      actual_json = {}
      if actual.kind_of?(Array) || actual.kind_of?(Hash)
        if actual.kind_of?(Array)
          actual.map do |e|
            if e.kind_of?(Hash)
              e.deep_stringify_keys!
            end
          end
        elsif actual.kind_of?(Hash)
          actual.deep_stringify_keys!
        end
        actual_json = actual
      elsif actual.kind_of?(String)
        if File.exist?(actual)
          actual_json = JSON.parse(File.read(actual))
        else
          assert false, "#{header}actual file path must exists, actual: #{actual}"
        end
      else
        assert false, "#{header}actual must a file path or a json object, actual: #{actual.inspect}"
      end

      assert_equal expected_json, actual_json, "#{header} json content must be equals"

    end

    def skip_timeout_with_ASAN

      if ENV['OG_ASAN_ENABLE'] == "1"

        # https://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg544042.html
        skip('ASAN false positive with pthread_cancel() - skipped')

      end

    end

  end

end
