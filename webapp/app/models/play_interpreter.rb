class PlayInterpreter
  include ActiveModel::Model
  include ActiveModel::Validations::Callbacks

  before_validation :set_defaults

  attr_accessor :ownername, :agentname, :text, :agent, :agents, :results

  validates_presence_of :ownername, :agentname, :text
  validates :text, byte_size: { maximum: 1024 * 8 }

  def proceed
    self.results = {}
    agents.each do |agent|
      # request_params = {
      #   format: "json",
      #   ownername: agent.owner.username,
      #   agentname: agent.agentname,
      #   agent_token: agent.api_token,
      #   verbose: 'false',
      #   sentence: @text
      # }
      # interpreter = Nlp::Interpret.new(request_params)
      # if interpreter.valid?
      #   data = interpreter.proceed
      #   @results[agent.id] = data[:body]
      # end
      self.results[agent.id] = fake_data
    end
  end


  private

  def set_defaults
    @agents  = [] if agents.nil?
    @results = {} if results.nil?
  end

  def fake_data
    {
      "interpretations" => [
        {
          "id" => "6b675028-8683-43b1-a131-f61a4a91cfa6",
          "slug" => "mathilde/villesfr/interpretations/cityfr_entities",
          "name" => "cityfr_entities",
          "score" => 1.0,
          "start_position" => 2,
          "end_position" => 7,
          "solution" => {
            "type" => "locations",
            "name" => "LILLE",
            "lat" => "50.633333",
            "lng" => "3.066667"
          }
        },
        {
          "id" => "6b675028-8683-43b1-a131-f61a4a91cfa6",
          "slug" => "mathilde/villesfr/interpretations/cityfr_entities",
          "name" => "cityfr_entities",
          "score" => 1.0,
          "start_position" => 60,
          "end_position" => 65,
          "solution" => {
            "type" => "locations",
            "name" => "PARIS",
            "lat" => "48.866667",
            "lng" => "2.333333"
          }
        },
        {
          "id" => "6b675028-8683-43b1-a131-f61a4a91cfa6",
          "slug" => "mathilde/villesfr/interpretations/cityfr_entities",
          "name" => "cityfr_entities",
          "score" => 1.0,
          "start_position" => 108,
          "end_position" => 112,
          "solution" => {
            "type" => "locations",
            "name" => "NICE",
            "lat" => "43.7",
            "lng" => "7.25",
          }
        },
        {
          "id" => "6b675028-8683-43b1-a131-f61a4a91cfa6",
          "slug" => "mathilde/villesfr/interpretations/cityfr_entities",
          "name" => "cityfr_entities",
          "score" => 1.0,
          "start_position" => 156,
          "end_position" => 166,
          "solution" => {
            "type" => "locations",
            "name" => "STRASBOURG",
            "lat": "48.583333",
            "lng": "7.75"
          }
        },
        {
          "id" => "6b675028-8683-43b1-a131-f61a4a91cfa6",
          "slug" => "mathilde/villesfr/interpretations/cityfr_entities",
          "name" => "cityfr_entities",
          "score" => 1.0,
          "start_position" => 230,
          "end_position" => 241,
          "solution" => {
            "type" => "locations",
            "name" => "MONTPELLIER",
            "lat" => "43.6",
            "lng" => "3.883333",
          }
        }
      ]
    }
  end


end
