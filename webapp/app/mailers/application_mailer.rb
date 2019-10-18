class ApplicationMailer < ActionMailer::Base
  default from: ENV.fetch("MAILER_SENDER") { 'support@viky.ai' }
  layout 'mailer'
end
