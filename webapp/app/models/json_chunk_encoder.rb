require 'yajl'

class JsonChunkEncoder

  FLUSH_THRESHOLD = 1_000

  SEPARATOR = ','.freeze

  def initialize(io)
    @io = io
    @need_separator = false
    @buffer = []
    @cache_buffer = Hash.new { |hash, key| hash[key] = [] }
  end

  def write_object(object, cache_key = nil)
    json = Yajl::Encoder.encode(object)
    @buffer << json
    @cache_buffer[cache_key] << json if cache_key.present?
    flush if @buffer.size > FLUSH_THRESHOLD
  end

  def write_value(key, value)
    @buffer << "\"#{key}\":\"#{value}\""
    flush if @buffer.size > FLUSH_THRESHOLD
  end

  def write_string(string)
    @buffer << string
    flush if @buffer.size > FLUSH_THRESHOLD
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

    def flush
      return if @buffer.blank?
      @io.write(SEPARATOR) if @need_separator
      @io.write(@buffer.join(SEPARATOR))
      @buffer.clear
      @need_separator = true
    end
end
