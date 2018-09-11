class StatisticsIndex

  attr_reader :name, :state, :version, :snapshot_id

  def self.from_template(template)
    StatisticsIndex.new template.index_name, template.state, template.version
  end

  def self.from_name(name)
    base_name = name.split('-')[1]
    state = name.split('-')[2]
    version = name.split('-')[3]
    id = name.split('-')[4]
    index = StatisticsIndex.new base_name, state, version, id
    if name.split('-').size == 6
      snapshot_id = name.split('-')[-2]
      index.snapshot_id = snapshot_id
    end
    index
  end

  def initialize(base_name, state, version, id = nil)
    @state = state
    @version = version.to_i
    uniq_id = id.present? ? id : SecureRandom.hex(4)
    @name = ['stats', base_name, state, version, uniq_id].join('-')
  end

  def snapshot_id=(id)
    @snapshot_id = id
    @name = (@name.split('-')[0..3] << id << @name.split('-')[4]).join('-')
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
