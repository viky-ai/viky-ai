# encoding: utf-8

module Nls

  class Package
    attr_reader :id
    attr_reader :slug
    attr_accessor :default_glue_distance
    attr_accessor :default_keep_order

    def initialize(slug, opts = {})

      @slug = slug

      # default values
      opts[:id] = UUIDTools::UUID.timestamp_create if !opts.has_key?(:id)

      @id = opts[:id]

      @interpretations_indexed_by_slug  = {}
      @interpretations_indexed_by_id = {}
    end

    def add_interpretation(new_interpretation)

      if !new_interpretation.kind_of? Interpretation
        raise "Interpretation  (#{new_interpretation}, #{new_interpretation.class}) added must a #{Interpretation.name} in package (#{@slug})"
      end

      if !@interpretations_indexed_by_slug[new_interpretation.slug].nil?
        raise "Same interpretation slug added (#{new_interpretation.slug}) in package (#{@slug})"
      end

      if !@interpretations_indexed_by_id[new_interpretation.id].nil?
        raise "Same interpretation id added (#{new_interpretation.id}) in package (#{@slug})"
      end

      @interpretations_indexed_by_slug[new_interpretation.slug] = new_interpretation
      @interpretations_indexed_by_id[new_interpretation.id] = new_interpretation
      new_interpretation.package = self
    end
    alias_method '<<', 'add_interpretation'

    def interpretation(slug_or_uuid)
      if slug_or_uuid.kind_of? UUIDTools::UUID
        @interpretations_indexed_by_id[slug_or_uuid]
      else
        @interpretations_indexed_by_slug[slug_or_uuid]
      end
    end
    alias_method '[]', 'interpretation'

    def to_h
      {
        "id" => @id.to_s,
        "slug" => @slug,
        "interpretations" => @interpretations_indexed_by_slug.values.map{|v| v.to_h}
      }
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

    def to_file(destination_dir)
      filename = "package_#{@slug}_#{@id}.json"
      File.open(File.join(File.expand_path(destination_dir), filename), "w") do |f|
        f.write(JSON.pretty_generate(self))
      end
    end

    def new_interpretation(text, opts = {} )
      # default values
      opts[:id] = UUIDTools::UUID.timestamp_create if !opts.has_key?(:id)

      id = opts[:id]

      solution = nil
      solution = opts[:solution] if opts.has_key?(:solution)

      scope = nil
      scope = opts[:scope] if opts.has_key?(:scope)

      interpretation = Interpretation.new(text, {id: id, scope: scope, solution: solution})

      add_interpretation(interpretation)

      interpretation
    end

  end
end
