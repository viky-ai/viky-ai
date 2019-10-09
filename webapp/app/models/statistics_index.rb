class StatisticsIndex

  attr_reader :base_name, :state, :version, :uid

  def self.from_template(template)
    index_name = template.index_patterns.split('-')[0..1].join('-')
    StatisticsIndex.new index_name, template.state, template.version
  end

  def self.from_name(name)
    base_name = name.split('-')[0..1].join('-')
    state = name.split('-')[2]
    version = name.split('-')[3]
    uid = name.split('-')[4]
    StatisticsIndex.new base_name, state, version, uid
  end

  def initialize(base_name, state, version, uid = nil)
    @base_name = base_name
    @state = state
    @version = version.to_i
    @uid = uid.present? ? uid : SecureRandom.hex(4)
  end

  def name
    [@base_name, @state, @version, @uid].join('-')
  end

  def need_reindexing?(template)
    @version != template.version
  end

  def active?
    @state == 'active'
  end

  def inactive?
    @state == 'inactive'
  end
end
