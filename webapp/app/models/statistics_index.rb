class StatisticsIndex

  attr_reader :base_name, :state, :version, :uid
  attr_accessor :snapshot_id

  def self.from_template(template)
    StatisticsIndex.new template.index_name, template.state, template.version
  end

  def self.from_name(name)
    base_name = name.split('-')[1]
    state = name.split('-')[2]
    version = name.split('-')[3]
    uid = name.split('-')[4]
    snapshot_id = name.split('-').size == 6 ? name.split('-')[-1] : nil
    StatisticsIndex.new base_name, state, version, uid, snapshot_id
  end

  def initialize(base_name, state, version, uid = nil, snapshot_id = nil)
    @base_name = base_name
    @state = state
    @version = version.to_i
    @uid = uid.present? ? uid : SecureRandom.hex(4)
    @snapshot_id = snapshot_id
  end

  def name
    name_parts = ['stats', @base_name, @state, @version, @uid]
    name_parts << @snapshot_id if @snapshot_id.present?
    name_parts.join('-')
  end

  def snapshot?
    @snapshot_id.present?
  end

  def need_reindexing?(template)
    @version != template.version
  end

  def active?
    @state == 'active'
  end
end
