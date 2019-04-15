require 'test_helper'

class JsonChunkEncoderTest < ActiveSupport::TestCase

  test 'Simple key/value object' do
    io = StringIO.new
    encoder = JsonChunkEncoder.new io
    encoder.wrap_object do
      encoder.write(key: 'foo', value: 'bar')
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
      encoder.write(key: 'foo1', value: 'bar1')
      encoder.write(key: 'foo2', value: 'bar2')
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
      encoder.write(object: { foo1: 'bar1' })
      encoder.write(object: { foo2: 'bar2' })
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
        encoder.write object: 'foo'
        encoder.write object: 'bar'
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
      encoder.write(key: 'foo', value: 'bar')
      encoder.wrap_array('my_array') do
        encoder.write(object: { id: '1', value: 'one' })
        encoder.write(object: { id: '2', value: 'two' })
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
end
