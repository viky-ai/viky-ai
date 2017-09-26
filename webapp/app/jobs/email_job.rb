class EmailJob < ApplicationJob
  queue_as :webapp_mailers

  def perform(*args)
    # Do something later
  end
end
