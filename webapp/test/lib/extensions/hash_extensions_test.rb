# frozen_string_literal: true

require 'test_helper'

class HashExtensionsTest < ActiveSupport::TestCase
  test 'flatten_by_keys example case' do
    h = { foo: { bar: { baz: 1 }, bay: [2, 3, 4] } }
    e = { 'foo.bar.baz': 1, 'foo.bay': [2, 3, 4] }
    assert_equal e, h.flatten_by_keys
  end

  test 'flatten_by_keys complexe case' do
    h = {
      key1: 'value1',
      key2: {
        key21: 'value21',
        key22: {
          key221: 'value221',
          key222: 'value222'
        },
        key23: 'value23'
      },
      key3: %w[
        value31
        value32
      ],
      key4: {
        key41: [
          'value411',
          'value412',
          %w[
            value413
            value414
          ]
        ],
        key42: 'value42'
      },
      key5: {
        key51: [
          'value511',
          {
            key512_skipped: 'value512'
          }
        ]
      }
    }

    e = {
      'key1': 'value1',
      'key2.key21': 'value21',
      'key2.key22.key221': 'value221',
      'key2.key22.key222': 'value222',
      'key2.key23': 'value23',
      'key3': %w[
        value31
        value32
      ],
      'key4.key41': %w[
        value411
        value412
        value413
        value414
      ],
      'key4.key42': 'value42',
      'key5.key51': ['value511']
    }

    assert_equal e, h.flatten_by_keys
  end
end
