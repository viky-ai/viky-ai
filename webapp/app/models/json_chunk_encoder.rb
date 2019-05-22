require 'yajl'

class JsonChunkEncoder

  COUNT_FLUSH_THRESHOLD = 1_000
  SIZE_FLUSH_THRESHOLD = 200_000

  SEPARATOR = ','.freeze

  def initialize(io)
    @io = io
    @need_separator = false
    @buffer = []
    @buffer_size = @buffer.size
    @cache_buffer = Hash.new { |hash, key| hash[key] = [] }
  end

  def write_object(object, cache_key = nil)
    json = Yajl::Encoder.encode(object)
    @cache_buffer[cache_key] << json if cache_key.present?
    buffered_write(json)
  end

  def write_value(key, value)
    buffered_write("\"#{key}\":\"#{value}\"")
  end

  def write_string(string)
    buffered_write(string)
  end

  def wrap_object(key = nil)
    flush
    @io.write(SEPARATOR) if @need_separator
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
    @io.write(SEPARATOR) if @need_separator
    @io.write("\"#{key}\":") if key.present?
    @io.write('[')
    @need_separator = false
    yield if block_given?
    flush
    @io.write(']')
    @need_separator = true
  end

  def withdraw_cache_payload(cache_key)
    return unless @cache_buffer.key? cache_key

    result = @cache_buffer[cache_key].join(SEPARATOR)
    @cache_buffer.delete(cache_key)
    result
  end

  private

    def buffered_write(json)
      @buffer << json
      @buffer_size += json.size
      buffer_full = @buffer.size > COUNT_FLUSH_THRESHOLD || @buffer_size > SIZE_FLUSH_THRESHOLD
      flush if buffer_full
    end


    def flush
      return if @buffer.blank?

      @io.write(SEPARATOR) if @need_separator
      @io.write(@buffer.join(SEPARATOR))
      @buffer.clear
      @buffer_size = @buffer.size
      @need_separator = true
    end
end
