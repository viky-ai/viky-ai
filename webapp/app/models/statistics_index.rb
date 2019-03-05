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
    if name.split('-').size == 6
      snapshot_id = name.split('-')[-2]
      uid = name.split('-')[-1]
    else
      snapshot_id = nil
    end
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
    if @snapshot_id.present?
      ['stats', @base_name, @state, @version, @snapshot_id, @uid].join('-')
    else
      ['stats', @base_name, @state, @version, @uid].join('-')
    end
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
