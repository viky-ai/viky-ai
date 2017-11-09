require 'test_helper'

class PackageTest < ActiveSupport::TestCase
  setup do
    @fixtures_layer_path = File.join(
      Rails.root, 'test', 'fixtures', 'models', 'nlp'
    )
  end

  test "A package update without options is not valid" do
    p = Nlp::Package.new
    res = p.update

    expected = {
      id: ["is empty"],
      agent_id: ["is empty"],
      slug: ["is empty"],
      interpretations: ["must be an array of interpretation objects, or an empty array"]
    }
    assert_equal expected, p.errors.messages
    assert !res
  end

  test "A package update with validated options generates an import file" do
    nlp_layer_packages_200 = JSON.parse(File.read(File.join(@fixtures_layer_path, "nlp_layer_packages_200.json")))
    Nlp::Package.any_instance.stubs('post_to_nlp').returns(nlp_layer_packages_200)

    opts = nlp_layer_packages_200.deep_symbolize_keys
    opts[:agent_id] = 'mybot'
    p = Nlp::Package.new(opts)
    res = p.update

    assert_not_nil p.data['id']
    assert_not_nil p.data['slug']
    assert_not_nil p.data['interpretations']
    assert res

    outfilename = File.join(Rails.root, 'import', "#{opts[:agent_id]}.json")
    assert File.exist?(outfilename)
  end

end
