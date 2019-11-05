class ExpressionProximity

  FORMULATION_PROXIMITIES = %w(glued accepts_punctuations very_close close far).freeze
  ENTITY_PROXIMITES = %w(glued very_close close far).freeze

  def initialize(proximity)
    @proximity = proximity
  end

  def get_distance
    case @proximity
      when "glued", "accepts_punctuations"
        0
      when "very_close"
        10
      when "close"
        20
      when "far"
        50
    end
  end

  def to_s
    @proximity.to_s
  end
end
