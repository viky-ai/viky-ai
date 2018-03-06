class EntityTermsParser

  def initialize(input)
    @input = input
    @output = nil
  end

  def proceed
    detect_lines
    detect_term_and_locale
    define_format
    @output
  end

  private

    def detect_lines
      @output = @input.split("\n").reject(&:blank?).uniq
    end

    def detect_term_and_locale
      @output = @output.collect { |line|
        term = ""
        locale = Locales::ANY
        if line.include?(":")
          if line.strip != ":"
            term, separator, locale = line.rpartition(":")
            locale = Locales::ANY if locale.blank?
            [term.strip, locale.strip]
          end
        else
          [line.strip, locale]
        end
      }
      .reject(&:nil?)
      .uniq
    end

    def define_format
      result = @output.collect { |array| { 'term' => array[0], 'locale' => array[1] } }
      @output = result.empty? ? nil : result
    end

end
