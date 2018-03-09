module Colorable
  extend ActiveSupport::Concern

  AVAILABLE_COLORS = %w[black red pink purple deep-purple indigo blue
                        light-blue cyan teal green light-green lime
                        yellow amber orange deep-orange brown].freeze

  included do
    before_create :set_color
  end


  private

    def set_color
      return if color.present?
      random_index = Random.new.rand(0..Colorable::AVAILABLE_COLORS.size - 1)
      self.color = Colorable::AVAILABLE_COLORS[random_index]
    end

end
