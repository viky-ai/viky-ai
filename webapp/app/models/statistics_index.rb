class StatisticsIndex

  attr_reader :base_name, :state, :version, :uid

  def self.from_template(template)
    StatisticsIndex.new template.index_name, template.state, template.version
  end

  def self.from_name(name)
    base_name = name.split('-')[1]
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
    ['stats', @base_name, @state, @version, @uid].join('-')
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
