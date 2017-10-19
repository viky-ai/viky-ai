class Api::V1::NlsController < Api::V1::ApplicationController

  def interpret
    @nls = Nls::Interpret.new(params)
    @nls.interpret
  end

end
