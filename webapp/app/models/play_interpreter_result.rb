class PlayInterpreterResult
  attr_accessor :status, :body

  def initialize(status, body)
    @status = status
    @body = body
  end

  def succeed?
    status == 200
  end

  def failed?
    !succeed?
  end

  def error_messages
    if failed?
      body[:errors].join(', ') unless body[:errors].nil?
    end
  end

  def interpretations_count
    succeed? ? body['interpretations'].size : 0
  end

  def interpretations_count_per_agent
    if succeed?
      body['interpretations'].group_by{ |i| i["package"] }.collect {|k, v| [k, v.size]}.to_h
    else
      {}
    end
  end

  def interpretations_count_for_agent(agent_id)
    body['interpretations'].select{|interpretation| interpretation["package"] == agent_id }.size
  end

  def slug_colors
    data = {}
    if succeed?
      body['interpretations'].collect{|i| i["slug"]}.uniq.each_with_index do |slug, i|
        data[slug] = colors[ i % colors.size ]
      end
    end
    data
  end


  private

    def colors
      [
        "pink",
        "indigo",
        "cyan",
        "light-green",
        "amber",
        "brown",
        "purple",
        "blue",
        "teal",
        "lime",
        "orange",
        "red",
        "deep-purple",
        "light-blue",
        "green",
        "deep-orange",
        "black"
      ]
    end

end
