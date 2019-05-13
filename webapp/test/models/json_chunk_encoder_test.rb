require 'test_helper'

class JsonChunkEncoderTest < ActiveSupport::TestCase

  test 'Simple key/value object' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.write_value('foo', 'bar')
    end
    expected = {
      foo: 'bar'
    }
    assert_equal expected.to_json, io.string
  end


  test 'Comma insertion check' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.write_value('foo1', 'bar1')
      encoder.write_value('foo2', 'bar2')
    end
    expected = {
      foo1: 'bar1',
      foo2: 'bar2'
    }
    assert_equal expected.to_json, io.string

    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.wrap_array('array1')
      encoder.wrap_array('array2')
    end
    expected = {
      array1: [],
      array2: []
    }
    assert_equal expected.to_json, io.string

    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.wrap_object('obj1')
      encoder.wrap_object('obj2')
    end
    expected = {
      obj1: {},
      obj2: {}
    }
    assert_equal expected.to_json, io.string

    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_array do
      encoder.write_object({ foo1: 'bar1' })
      encoder.write_object({ foo2: 'bar2' })
    end
    expected = [
      { foo1: 'bar1' },
      { foo2: 'bar2' }
    ]
    assert_equal expected.to_json, io.string
  end


  test 'Simple array object' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.wrap_array('my_array') do
        encoder.write_object('foo')
        encoder.write_object('bar')
      end
    end
    expected = {
      my_array: ['foo', 'bar']
    }
    assert_equal expected.to_json, io.string
  end


  test 'Composed object' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.write_value('foo', 'bar')
      encoder.wrap_array('my_array') do
        encoder.write_object({ id: '1', value: 'one' })
        encoder.write_object({ id: '2', value: 'two' })
      end
    end
    expected = {
      foo: 'bar',
      my_array: [{
        id: '1',
        value: 'one'
      }, {
        id: '2',
        value: 'two'
      }]
    }
    assert_equal expected.to_json, io.string
  end


  test 'Build a cache payload' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.wrap_array('my_array') do
        encoder.write_object({ id: 1, value: 'one' }, 'cache_1')
        encoder.write_object({ id: 2, value: 'two' }, 'cache_1')
        encoder.write_object({ id: 3, value: 'three' }, 'cache_2')
      end
    end
    expected = {
      my_array: [{
        id: 1,
        value: 'one'
      }, {
        id: 2,
        value: 'two'
      }, {
        id: 3,
        value: 'three'
      }]
    }
    assert_equal expected.to_json, io.string
    assert_equal '{"id":1,"value":"one"},{"id":2,"value":"two"}', encoder.withdraw_cache_payload('cache_1')
    assert_nil encoder.withdraw_cache_payload('cache_1')
    assert_equal '{"id":3,"value":"three"}', encoder.withdraw_cache_payload('cache_2')
    assert_nil encoder.withdraw_cache_payload('foo bar')
  end

  test 'Write raw string to IO' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.wrap_array('my_array') do
        encoder.write_string('{"id":1,"value":"one"},{"id":2,"value":"two"}')
        encoder.write_string('{"id":3,"value":"three"}')
      end
    end
    expected = {
      my_array: [{
        id: 1,
        value: 'one'
      }, {
        id: 2,
        value: 'two'
      }, {
        id: 3,
        value: 'three'
      }]
    }
    assert_equal expected.to_json, io.string
  end
end
