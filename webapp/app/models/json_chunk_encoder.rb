class JsonChunkEncoder

  def initialize(io)
    @io = io
    @need_separator = false
  end

  def write(options)
    @io.write(',') if @need_separator
    if options[:object].present?
      @io.write(options[:object].to_json)
    else
      @io.write("\"#{options[:key]}\":\"#{options[:value]}\"")
    end
    @need_separator = true
  end

  def wrap_object(key = nil)
    @io.write(',') if @need_separator
    @io.write("\"#{key}\":") if key.present?
    @io.write('{')
    @need_separator = false
    yield if block_given?
    @io.write('}')
    @need_separator = true
  end

  def wrap_array(key = nil)
    @io.write(',') if @need_separator
    @io.write("\"#{key}\":") if key.present?
    @io.write('[')
    @need_separator = false
    yield if block_given?
    @io.write(']')
    @need_separator = true
  end
end
