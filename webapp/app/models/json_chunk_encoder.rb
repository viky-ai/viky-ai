require 'yajl'

class JsonChunkEncoder

  FLUSH_THRESHOLD = 1000

  def initialize(io)
    @io = io
    @need_separator = false
    @buffer = []
  end

  def write(options)
    if options[:object].present?
      @buffer << Yajl::Encoder.encode(options[:object])
    else
      @buffer << "\"#{options[:key]}\":\"#{options[:value]}\""
    end
    flush if @buffer.size > FLUSH_THRESHOLD
  end

  def wrap_object(key = nil)
    flush
    @io.write(',') if @need_separator
    @io.write("\"#{key}\":") if key.present?
    @io.write('{')
    @need_separator = false
    yield if block_given?
    flush
    @io.write('}')
    @need_separator = true
  end

  def wrap_array(key = nil)
    flush
    @io.write(',') if @need_separator
    @io.write("\"#{key}\":") if key.present?
    @io.write('[')
    @need_separator = false
    yield if block_given?
    flush
    @io.write(']')
    @need_separator = true
  end

  private

    def flush
      return if @buffer.blank?
      @io.write(',') if @need_separator
      @io.write(@buffer.join(','))
      @buffer = []
      @need_separator = true
    end
end
