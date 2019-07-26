require 'fileutils'

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

    def ruby_log_file
      "ruby_test.log"
    end

    def ruby_log_file_backup
      "ruby_test.log2"
    end

    def manage_log_file_size
      if( (File.stat(File.join(pwd, ruby_log_file)).size) > 2000000 )
        File.rename(File.join(pwd, ruby_log_file), File.join(pwd, ruby_log_file_backup))
      end
    end

    def ruby_log_append(filename, sentence)
      FileUtils.touch(File.join(pwd, ruby_log_file))
      manage_log_file_size
      File.open(File.join(pwd, ruby_log_file),"a") do |f|
        f.puts("#{Time.now.utc.iso8601}: #{filename}: #{sentence}")
      end
    end

    def json_interpret_body(package, sentence, opts = {})

      default_opt = {
        locale: Interpretation.default_locale,
        explain: false,
        now: nil,
        primary_package: nil,
        show_private: nil,
        spellchecking: nil,
        enable_list: false
      }
      opts = default_opt.merge(opts)

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

      primary_package = opts[:primary_package]
      unless primary_package.nil?
        primary_package_id = primary_package
        primary_package_id = primary_package.id.to_s if primary_package.kind_of? Package
      end

      spellchecking = opts[:spellchecking]
      spellchecking_valid_values = [nil, :inactive, :low, :medium, :high]
      unless  spellchecking_valid_values.include?(spellchecking)
        raise "spellchecking: not in #{spellchecking_valid_values}"
      end

      request['sentence'] = sentence
      request['primary-package'] = primary_package_id unless primary_package_id.nil?
      request['spellchecking'] = spellchecking        unless spellchecking.nil?
      request['Accept-Language'] = opts[:locale]      unless opts[:locale].nil?
      request['now'] = opts[:now]                     unless opts[:now].nil?
      request['show-explanation'] = true              if     opts[:explain]
      request['show-private'] = true                  if     opts[:show_private]
      request['enable-list'] = false
      request['enable-list'] = true                   if     opts[:enable_list]

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

    def create_regex_package
      package = Package.new("regex_package")

      email = package.new_interpretation("email", { scope: "private" })
      email << Expression.new("@{email}", aliases: { email: Alias.regex("[a-z]+@[a-z]+\.[a-z]+") }, solution: "`{ email: email }`")

      emails = package.new_interpretation("emails", { scope: "public" })
      emails << Expression.new("@{email}", aliases: { email: email })
      emails << Expression.new("@{email} @{emails}", aliases: { email: email, emails: emails })

      @available_packages[package.slug] = package

      package
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

    def create_numbers_package
      package = Package.new("numbers")

      letters = package.new_interpretation("letters")
      letters << Expression.new("a", solution: "a")
      letters << Expression.new("b", solution: "b")

      numbers = package.new_interpretation("numbers")
      numbers << Expression.new("@{number}", aliases: { number: Alias.number })

      titi = package.new_interpretation("titi")
      titi << Expression.new("titi")

      toto = package.new_interpretation("toto")
      toto << Expression.new("toto")

      combination1 = package.new_interpretation("combination1")
      combination1 << Expression.new("@{number} @{toto}", aliases: { number: Alias.number, toto: toto })

      combination2 = package.new_interpretation("combination2")
      combination2 << Expression.new("@{number} @{titi}", aliases: { number: Alias.number, titi: titi })

      combination = package.new_interpretation("combination")
      combination << Expression.new("@{combination2}", aliases: { combination2: combination2 })
      combination << Expression.new("@{combination1}", aliases: { combination1: combination1 })

      combinations = package.new_interpretation("combinations")
      combinations << Expression.new("@{combinations} @{combination}", aliases: {combinations: combinations, combination: combination})
      combinations << Expression.new("@{combination}", aliases: {combination: combination})


      numbers_list = package.new_interpretation("numbers_list")
      numbers_list << Expression.new("@{number}", aliases: {number: numbers})
numbers_list << Expression.new("@{number} @{numbers}", aliases: {number: numbers, numbers: numbers_list})

      complex_number = package.new_interpretation("complex_number")
      complex_number << Expression.new("@{letter1} @{number} @{letter2}", aliases: { letter1: letters, number: Alias.number, letter2: letters})

      @available_packages[package.slug] = package

      package
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
      actual = {}
      if sentence.kind_of? Array
        actual = check_interpret_array(sentence, expected)
      elsif sentence.kind_of? Hash
        actual = check_interpret_hash(sentence, expected)
      else
        actual = check_interpret_sentence(sentence, false, expected)
      end
      actual
    end

    def check_interpret_array(sentence, expected)

      raise "expected must be an Array"   if !expected.kind_of? Array
      raise "expected must not be empty" if expected.empty?
      raise "expected must have as many elements as sentence" if expected.length != sentence.length

      # on initialise tous les champs en tableaux pour pouvoir ensuite les remplir

      globaldebug = false
      debug = []
      now = []
      packages = []
      primary_package = []
      show_private = []
      request = []

      0.upto(sentence.length-1) do |i|

        enable_list = false
        sentence_str = nil
        if sentence[i].kind_of? Hash
          sentence_str = nil
          sentence_str = sentence[i][:sentence] if sentence[i].has_key?(:sentence)
          if sentence_str.nil?
            assert false, "no sentence in the sentence hash"
          end
          enable_list = sentence[i][:enable_list] if sentence[i].has_key?(:enable_list)
        else
          sentence_str = sentence[i]
        end

        debug[i] = expected[i][:debug]
        globaldebug = globaldebug || debug[i]

        now[i] = expected[i][:now]

        packages[i] = "*"
        packages[i] = expected[i][:packages] if expected[i].has_key?(:packages)
        primary_package[i] = expected[i][:primary_package]

        show_private[i] = false
        show_private[i] = expected[i][:show_private] if expected[i].has_key?(:show_private)

        explain = false
        explain = expected[:explain] if expected.has_key?(:explain)

        spellchecking = nil
        spellchecking = expected[:spellchecking] if expected.has_key?(:spellchecking)

        opts = {
          locale: Interpretation.default_locale,
          explain: explain,
          now: now[i],
          primary_package: primary_package[i],
          show_private: show_private[i],
          spellchecking: spellchecking,
          enable_list: enable_list
        }

        request << json_interpret_body(packages[i], sentence[i], opts)
      end

      ap expected if globaldebug
      ap request if globaldebug
      # execution de la requete
      actual = Nls.interpret(request)
      ap actual if globaldebug

      assert_kind_of Array, actual, "Actual answer is not an Array : #{actual.inspect}"
      actual.each do |actual_element|
        assert_kind_of Hash, actual_element, "Actual answer element is not an hash : #{actual_element.inspect}"
        assert_kind_of Array, actual_element['interpretations'], "Actual answer['interpretations'] is not an Array : #{actual_element['interpretations']}"
      end

      0.upto(expected.length-1) do |i|
        if (expected[i].has_key?(:interpretations) && expected[i][:interpretations].kind_of?(Array) && expected[i][:interpretations].empty?) ||
        (expected[i].has_key?(:interpretation)  && expected[i][:interpretation].nil?)
          assert actual[i]['interpretations'].empty?, "Actual answer should not match on any interpretation"
          # skip other assert
          return actual[i]
        end

        assert !actual[i]['interpretations'].empty?, "Actual answer did not match on any interpretation"

        match_intepretation = actual[i]['interpretations'].first

        if expected[i].has_key?(:warnings)

          assert_kind_of Array, actual[i]['warnings'], "Actual answer['warnings'] is not an Array : #{actual[i].inspect}"

          expected_warnings = expected[i][:warnings]

          expected_warning_found = false
          actual[i]['warnings'].each do |warning|
            if warning.include?(expected_warnings)
              expected_warning_found = true
              break
            end
          end

          assert expected_warning_found, "Actual answer['warnings'] must contains \"#{expected_warnings}\": \n#{JSON.generate(actual[i]['warnings'])}"

        else
          if !actual[i]['warnings'].nil?
            assert false, "Actual answer must be without warnings :\n#{actual[i]['warnings'].join("\n")}"
          end
        end


        if expected[i].has_key?(:interpretations)
          expected_interpretations = expected[i][:interpretations]

          match_intepretations_slug = []
          actual[i]['interpretations'].each do |match_intepretation_local|
            match_intepretations_slug << match_intepretation_local['slug']
          end

          assert_equal expected_interpretations, match_intepretations_slug, "match on wrong interpretation"
        end

        if expected[i].has_key?(:interpretation)
          expected_interpretation = expected[i][:interpretation]
          slug_match = match_intepretation['slug'] == expected_interpretation || match_intepretation['id'] == expected_interpretation
          assert slug_match, "match on wrong interpretation : \n  actual: slug:#{match_intepretation['slug']}, id:#{match_intepretation['id']}\n  expected: #{expected_interpretation}"
        end

        if expected[i].has_key?(:solution)
          expected_solution = expected[i][:solution]
          if expected_solution.kind_of?(Hash)
            expected_solution.deep_stringify_keys!
          elsif expected_solution.kind_of?(Array)
            expected_solution.each do |h|
              if h.kind_of?(Hash)
                h.deep_stringify_keys!
              end
            end
          end

          if expected_solution.nil?
            assert_nil match_intepretation['solution'], "Matched on unexpected solution (nil)"
          else
            assert_equal expected_solution, match_intepretation['solution'], "Matched on unexpected solution"
          end
        end

        if expected[i].has_key?(:score)
          expected_score = expected[i][:score]
          assert_equal expected_score, match_intepretation['score'], "Matched on wrong score"
        end

      end



      return actual

    end

    def check_interpret_hash(sentence, expected)
      enable_list = false
      enable_list = sentence[:enable_list] if sentence.has_key?(:enable_list)
      sentence_str = nil
      sentence_str = sentence[:sentence] if sentence.has_key?(:sentence)
      if sentence_str.nil?
        assert false, "no sentence in the sentence hash"
      end
      if enable_list
        return check_interpret_sentence_list(sentence_str, enable_list, expected)
      else
        return check_interpret_sentence(sentence_str, enable_list, expected)
      end
    end

    def check_interpret_sentence(sentence, enable_list, expected)

      raise "expected must be an Hash"   if !expected.kind_of? Hash
      raise "expected must not be empty" if expected.empty?

      debug = expected[:debug]

      now = expected[:now]

      locale = Interpretation.default_locale
      locale = expected[:locale] if expected.has_key?(:locale)

      packages = "*"
      packages = expected[:packages] if expected.has_key?(:packages)
      primary_package = expected[:primary_package]

      show_private = false
      show_private = expected[:show_private] if expected.has_key?(:show_private)

      explain = false
      explain = expected[:explain] if expected.has_key?(:explain)

      spellchecking = nil
      spellchecking = expected[:spellchecking] if expected.has_key?(:spellchecking)

      opts = {
        locale: locale,
        explain: explain,
        now: now,
        primary_package: primary_package,
        show_private: show_private,
        spellchecking: spellchecking,
        enable_list: enable_list
      }

      ap expected if debug
      # creation et exécution de la requete
      request = json_interpret_body(packages, sentence, opts)
      ap request if debug
      actual = Nls.interpret(request)
      ap actual if debug

      assert_kind_of Hash, actual, "Actual answer is not an Hash : #{actual.inspect}"
      assert_kind_of Array, actual['interpretations'], "Actual answer['interpretations'] is not an Array : #{actual['interpretations']}"

      if (expected.has_key?(:interpretations) && expected[:interpretations].kind_of?(Array) && expected[:interpretations].empty?) ||
         (expected.has_key?(:interpretation) && expected[:interpretation].nil?)
        assert actual['interpretations'].empty?, "Actual answer should not match on any interpretation. Matching #{actual['interpretations']}"
        # skip other assert
        return actual
      end

      assert !actual['interpretations'].empty?, "Actual answer did not match on any interpretation"

      match_intepretation = actual['interpretations'].first

      if expected.has_key?(:explanation)
        assert match_intepretation['explanation'], "Actual answer has no explanation"
        expected_explanation = expected[:explanation]
        actual_explanation = match_intepretation['explanation']

        if expected_explanation.has_key?(:scores)
          expected_score = expected_explanation[:scores]
          assert actual_explanation['scores'], "Actual explanation has no score"
          assert_equal expected_score, actual_explanation['scores'], "Matched on wrong score"
        end


        if expected_explanation.has_key?(:highlight)

          expected_highlight = expected_explanation[:highlight]
          assert actual_explanation['highlight'], "Actual explanation has no highlight"
          check_highlight expected_highlight, actual_explanation['highlight']
        end

        if expected_explanation.has_key?(:expression)
          assert actual_explanation['expression'], "Actual explanation has no expression"
          expected_expression = expected_explanation[:expression]
          check_explanation_expression_value = check_explanation_expression expected_expression, actual_explanation['expression']
          assert check_explanation_expression_value, "wrong match on expression\nexpected: #{expected_expression}\nactual: #{actual_explanation['expression']}"
        end
      end

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
        assert slug_match, "match on wrong interpretation : \n  actual: slug:#{match_intepretation['slug']}, id:#{match_intepretation['id']}\n  expected: #{expected_interpretation}"
      end

      if expected.has_key?(:solution)
        expected_solution = expected[:solution]
        if expected_solution.kind_of?(Hash)
          expected_solution.deep_stringify_keys!
        elsif expected_solution.kind_of?(Array)
          expected_solution.each do |h|
            if h.kind_of?(Hash)
              h.deep_stringify_keys!
            end
          end
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

    def check_interpret_sentence_list(sentence, enable_list, expected)

      raise "expected must be an Array"   if !expected.kind_of? Array
      raise "expected must not be empty" if expected.empty?

      #TODO
      # pas d'options pour le moment, refactoring à faire

      debug = false
      # debug = expected[:debug] if expected.has_key?(:debug)

      now = nil
      # now = expected[:now] if expected.has_key?(:now)

      locale = Interpretation.default_locale
      # locale = expected[:locale] if expected.has_key?(:locale)

      packages = "*"
      # packages = expected[:packages] if expected.has_key?(:packages)

      primary_package = nil
      # primary_package = expected[:primary_package]

      show_private = false
      # show_private = expected[:show_private] if expected.has_key?(:show_private)

      explain = false
      # explain = expected[:explain] if expected.has_key?(:explain)

      spellchecking = nil
      # spellchecking = expected[:spellchecking] if expected.has_key?(:spellchecking)

      opts = {
        locale: locale,
        explain: explain,
        now: now,
        primary_package: primary_package,
        show_private: show_private,
        spellchecking: spellchecking,
        enable_list: enable_list
      }

      ap expected if debug
      # creation et exécution de la requete
      request = json_interpret_body(packages, sentence, opts)
      ap request if debug
      actual = Nls.interpret(request)
      ap actual if debug

      assert_kind_of Hash, actual, "Actual answer is not an Hash : #{actual.inspect}"
      assert_kind_of Array, actual['interpretations'], "Actual answer['interpretations'] is not an Array : #{actual['interpretations']}"

      raise "expected must be an Array as sentence is a list"   if !expected.kind_of? Array

      assert !actual['interpretations'].empty?, "Actual answer did not match on any interpretation"

      global_match = true

      expected.each do |expected_element|
        actual['interpretations'].each do |interpretation|
          match_interpretation = true
          match_solution = true
          match_score = true
          if expected_element.has_key?(:interpretations)
            match_interpretation = false
            expected_interpretation = expected_element[:interpretations]
            match_interpretation = true if expected_interpretation == interpretation['slug']

            if expected_element.has_key?(:solution)
              match_solution = solution_match(expected_element, interpretation['solution'])

              if expected_element.has_key?(:score)
                match_score = false
                expected_score = expected_element[:solution]
                match_score = true if expected_score == interpretation['score']
              end
            end
          end
          if expected_element.has_key?(:solution)
            match_solution = solution_match(expected_element, interpretation['solution'])

            if expected_element.has_key?(:score)
              match_score = false
              expected_score = expected_element[:solution]
              match_score = true if expected_score == interpretation['score']
            end
          end
          if expected_element.has_key?(:score)
            match_score = false
            expected_score = expected_element[:solution]
            match_score = true if expected_score == interpretation['score']
          end

          global_match = match_interpretation && match_solution && match_score
          break if global_match
        end
        assert global_match , "no match found in returned interpretations for expected result #{expected_element}"
      end

      return actual

    end

    def solution_match(expected, actual)
      is_match = false
      if expected.has_key?(:solution)
        expected_solution = expected[:solution]
        if expected_solution.kind_of?(Hash)
          expected_solution.deep_stringify_keys!
        elsif expected_solution.kind_of?(Array)
          expected_solution.each do |h|
            if h.kind_of?(Hash)
              h.deep_stringify_keys!
            end
          end
        end

        if expected_solution.nil?
          assert_nil actual, "Matched on unexpected solution (nil)"
        else
          is_match = true if expected_solution == actual
        end
      end
      is_match
    end

    def check_highlight(expected_highlight, actual_highlight)
      if expected_highlight.has_key?(:summary)
        assert actual_highlight['summary'], "Actual explanation has no summary"
        expected_summary = expected_highlight[:summary]
        assert_equal expected_summary, actual_highlight['summary'], "Matched on wrong summary"
      end

      if expected_highlight.has_key?(:words)
        assert actual_highlight['words'], "Actual explanation has no words"
        expected_words = expected_highlight[:words]
        assert_kind_of Array, expected_words, "expected Words is not an array"
        assert_kind_of Array, actual_highlight['words'], "actual Words is not an array"
        expected_words.each do |expected_word|
          return_value = false
          actual_highlight['words'].each do |actual_word|
            return_value = true if check_highlight_word(expected_word, actual_word)
          end
          assert return_value, "match on wrong word\nexpected word: #{expected_word}\nwords in answer: #{actual_highlight['words']}"
        end
      end
    end

    def check_highlight_word(expected_word, actual_word)

      if expected_word.has_key?(:word)
        if(!actual_word['word'].nil?)
          if expected_word[:word] != actual_word['word']
            return false
          end
        else
          return false
        end
      end

      if expected_word.has_key?(:match)
        if(!actual_word['match'].nil?)
          if !check_highlight_match_or_parent(expected_word[:match], actual_word['match'])
            return false
          end
        else
          return false
        end
      end

      if expected_word.has_key?(:is_any)
        if(!actual_word['is_any'].nil?)
          if expected_word[:is_any]==true &&  actual_word['is_any']==true
            return true
          end
        else
          return false
        end
      end

      return true
    end

    def check_highlight_match_or_parent(expected_match, actual_match)

      if expected_match.has_key?(:expression)
        if(!actual_match['expression'].nil?)
          if expected_match[:expression] != actual_match['expression']
            return false
          end
        else
          return false
        end
      end

      if expected_match.has_key?(:interpretation_slug)
        if(!actual_match['interpretation_slug'].nil?)
          if expected_match[:interpretation_slug] != actual_match['interpretation_slug']
            return false
          end
        else
          return false
        end
      end

      if expected_match.has_key?(:parent)
        if(!actual_match['parent'].nil?)
          if !check_highlight_match_or_parent(expected_match[:parent],actual_match['parent'])
            return false
          end
        else
          return false
        end
      end

      return true
    end

    def check_explanation_expression(expected_expression, actual_expression)

      if expected_expression.has_key?(:text)
        if(!actual_expression['text'].nil?)
          if expected_expression[:text] != actual_expression['text']
            return false
          end
        else
          return false
        end
      end

      if expected_expression.has_key?(:slug)
        if(!actual_expression['slug'].nil?)
          if expected_expression[:slug] != actual_expression['slug']
            return false
          end
        else
          return false
        end
      end

      if expected_expression.has_key?(:highlight)
        if(!actual_expression['highlight'].nil?)
          if expected_expression[:highlight] != actual_expression['highlight']
            return false
          end
        else
          return false
        end
      end

      if expected_expression.has_key?(:scores)
        if(!actual_expression['scores'].nil?)
          if expected_expression[:scores] != actual_expression['scores']
            return false
          end
        else
          return false
        end
      end

      if expected_expression.has_key?(:word)
        if(!actual_expression['word'].nil?)
          if expected_expression[:word] != actual_expression['word']
            return false
          end
        else
          return false
        end
      end

      if expected_expression.has_key?(:expressions)
        if(!actual_expression['expressions'].nil?)
          if(actual_expression['expressions'].kind_of?(Array))
            expected_expression[:expressions].each do |expected_sub_expression|
              expected_sub_expression_matched = false
              actual_expression['expressions'].each do |actual_sub_expression|
                expected_sub_expression_matched = true if(check_explanation_expression(expected_sub_expression, actual_sub_expression))
              end
              return false if(!expected_sub_expression_matched)
            end
          else
            return false
          end
        else
          return false
        end
      end

      return true
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
