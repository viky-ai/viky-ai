# frozen_string_literal: true

# Extention of Hash
class Hash
  # Return a new {Hash} with only one deep level
  #
  # Note: if the hash contains a hash in a nested array this value are skipped.
  #
  # @example
  #   h = { foo: { bar: { baz: 1 }, bay: [2, 3, 4] } }
  #   h.flatten_by_keys
  #   #=> { 'foo.bar.baz': 1, 'foo.bay': [2, 3, 4] }
  #
  # @return [Hash]
  def flatten_by_keys
    h = {}
    each do |(k, v)|
      if v.is_a? Hash
        v.flatten_by_keys.each { |nk, nv| h["#{k}.#{nk}".to_sym] = nv }
      elsif v.is_a? Array
        # skip hash nested in an array
        h[k.to_sym] = v.flat_map { |e| e.is_a?(Hash) ? nil : e }.compact
      else
        h[k.to_sym] = v
      end
    end
    h
  end
end
