# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 20190412074505) do

  # These are extensions that must be enabled in order to support this database
  enable_extension "plpgsql"
  enable_extension 'pgcrypto' unless extension_enabled?('pgcrypto') # this is usefull for kubernetes build side

  create_table "agent_arcs", force: :cascade do |t|
    t.uuid "source_id"
    t.uuid "target_id"
    t.index ["source_id", "target_id"], name: "index_agent_arcs_on_source_id_and_target_id", unique: true
  end

  create_table "agent_regression_checks", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "sentence"
    t.string "language"
    t.datetime "now"
    t.uuid "agent_id"
    t.jsonb "expected"
    t.jsonb "got"
    t.integer "state", default: 0
    t.integer "position", default: 0
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.integer "spellchecking"
    t.index ["agent_id"], name: "index_agent_regression_checks_on_agent_id"
  end

  create_table "agents", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "name"
    t.text "description"
    t.string "agentname"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.uuid "owner_id"
    t.string "color", default: "black"
    t.text "image_data"
    t.string "api_token"
    t.integer "visibility", default: 0
    t.jsonb "source_agent"
    t.datetime "nlp_updated_at"
    t.jsonb "locales"
    t.index ["api_token"], name: "index_agents_on_api_token", unique: true
    t.index ["owner_id", "agentname"], name: "index_agents_on_owner_id_and_agentname", unique: true
  end

  create_table "bots", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "name"
    t.string "endpoint"
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.boolean "wip_enabled", default: true
    t.index ["agent_id"], name: "index_bots_on_agent_id"
  end

  create_table "chat_sessions", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.uuid "user_id"
    t.uuid "bot_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string "locale", default: "en-US"
    t.index ["bot_id"], name: "index_chat_sessions_on_bot_id"
    t.index ["user_id"], name: "index_chat_sessions_on_user_id"
  end

  create_table "chat_statements", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.integer "speaker"
    t.integer "nature", default: 0
    t.jsonb "content"
    t.uuid "chat_session_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["chat_session_id"], name: "index_chat_statements_on_chat_session_id"
  end

  create_table "entities", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.text "solution"
    t.boolean "auto_solution_enabled", default: true
    t.jsonb "terms"
    t.uuid "entities_list_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.integer "position", default: 0
    t.text "searchable_terms"
    t.index ["entities_list_id", "position"], name: "index_entities_on_entities_list_id_and_position", unique: true
    t.index ["entities_list_id"], name: "index_entities_on_entities_list_id"
    t.index ["terms"], name: "index_entities_on_terms", using: :gin
  end

  create_table "entities_imports", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.text "file_data"
    t.integer "mode"
    t.uuid "entities_list_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.integer "status", default: 0
    t.integer "duration", default: 0
    t.integer "filesize", default: 0
    t.uuid "user_id"
    t.index ["entities_list_id"], name: "index_entities_imports_on_entities_list_id"
  end

  create_table "entities_lists", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "listname"
    t.integer "position", default: 0
    t.string "color"
    t.text "description"
    t.integer "visibility", default: 0
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.integer "proximity", default: 0
    t.integer "entities_count", default: 0, null: false
    t.index ["agent_id"], name: "index_entities_lists_on_agent_id"
  end

  create_table "favorite_agents", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.uuid "user_id"
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["agent_id"], name: "index_favorite_agents_on_agent_id"
    t.index ["user_id", "agent_id"], name: "index_favorite_agents_on_user_id_and_agent_id", unique: true
    t.index ["user_id"], name: "index_favorite_agents_on_user_id"
  end

  create_table "friendly_id_slugs", force: :cascade do |t|
    t.string "slug", null: false
    t.uuid "sluggable_id", null: false
    t.string "sluggable_type", limit: 50
    t.string "scope"
    t.datetime "created_at"
    t.index ["slug", "sluggable_type", "scope"], name: "index_friendly_id_slugs_on_slug_and_sluggable_type_and_scope", unique: true
    t.index ["slug", "sluggable_type"], name: "index_friendly_id_slugs_on_slug_and_sluggable_type"
    t.index ["sluggable_id"], name: "index_friendly_id_slugs_on_sluggable_id"
    t.index ["sluggable_type"], name: "index_friendly_id_slugs_on_sluggable_type"
  end

  create_table "intents", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "intentname"
    t.text "description"
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.integer "position", default: 0
    t.string "color"
    t.integer "visibility", default: 0
    t.index ["agent_id"], name: "index_intents_on_agent_id"
    t.index ["intentname", "agent_id"], name: "index_intents_on_intentname_and_agent_id", unique: true
  end

  create_table "interpretation_aliases", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "aliasname", null: false
    t.integer "position_start"
    t.integer "position_end"
    t.uuid "interpretation_id"
    t.uuid "interpretation_aliasable_id"
    t.integer "nature", default: 0
    t.boolean "is_list", default: false
    t.boolean "any_enabled", default: false
    t.string "interpretation_aliasable_type"
    t.text "reg_exp"
    t.index ["interpretation_aliasable_type", "interpretation_aliasable_id"], name: "index_ialiases_on_ialiasable_type_and_ialiasable_id"
    t.index ["interpretation_id"], name: "index_interpretation_aliases_on_interpretation_id"
  end

  create_table "interpretations", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "expression"
    t.uuid "intent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string "locale"
    t.integer "position", default: 0
    t.boolean "keep_order", default: false
    t.text "solution"
    t.boolean "auto_solution_enabled", default: true
    t.integer "proximity", default: 3
    t.index ["intent_id"], name: "index_interpretations_on_intent_id"
  end

  create_table "memberships", force: :cascade do |t|
    t.uuid "user_id"
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string "rights", default: "show"
    t.index ["agent_id", "user_id"], name: "index_memberships_on_agent_id_and_user_id", unique: true
  end

  create_table "readmes", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.text "content"
    t.uuid "agent_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["agent_id"], name: "index_readmes_on_agent_id"
  end

  create_table "users", id: :uuid, default: -> { "gen_random_uuid()" }, force: :cascade do |t|
    t.string "email", default: "", null: false
    t.string "encrypted_password", default: "", null: false
    t.string "reset_password_token"
    t.datetime "reset_password_sent_at"
    t.datetime "remember_created_at"
    t.integer "sign_in_count", default: 0, null: false
    t.datetime "current_sign_in_at"
    t.datetime "last_sign_in_at"
    t.inet "current_sign_in_ip"
    t.inet "last_sign_in_ip"
    t.string "confirmation_token"
    t.datetime "confirmed_at"
    t.datetime "confirmation_sent_at"
    t.string "unconfirmed_email"
    t.integer "failed_attempts", default: 0, null: false
    t.string "unlock_token"
    t.datetime "locked_at"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.boolean "admin", default: false
    t.string "invitation_token"
    t.datetime "invitation_created_at"
    t.datetime "invitation_sent_at"
    t.datetime "invitation_accepted_at"
    t.integer "invitation_limit"
    t.string "invited_by_type"
    t.uuid "invited_by_id"
    t.integer "invitations_count", default: 0
    t.string "username"
    t.string "name"
    t.text "bio"
    t.text "image_data"
    t.jsonb "ui_state", default: {}
    t.index ["confirmation_token"], name: "index_users_on_confirmation_token", unique: true
    t.index ["email"], name: "index_users_on_email", unique: true
    t.index ["invitation_token"], name: "index_users_on_invitation_token", unique: true
    t.index ["invitations_count"], name: "index_users_on_invitations_count"
    t.index ["invited_by_id"], name: "index_users_on_invited_by_id"
    t.index ["invited_by_type", "invited_by_id"], name: "index_users_on_invited_by_type_and_invited_by_id"
    t.index ["reset_password_token"], name: "index_users_on_reset_password_token", unique: true
    t.index ["unlock_token"], name: "index_users_on_unlock_token", unique: true
    t.index ["username"], name: "index_users_on_username", unique: true
  end

  add_foreign_key "agent_arcs", "agents", column: "source_id", on_delete: :cascade
  add_foreign_key "agent_arcs", "agents", column: "target_id", on_delete: :cascade
  add_foreign_key "agent_regression_checks", "agents", on_delete: :cascade
  add_foreign_key "agents", "users", column: "owner_id"
  add_foreign_key "bots", "agents", on_delete: :cascade
  add_foreign_key "chat_sessions", "bots", on_delete: :cascade
  add_foreign_key "chat_sessions", "users", on_delete: :cascade
  add_foreign_key "chat_statements", "chat_sessions", on_delete: :cascade
  add_foreign_key "entities", "entities_lists", on_delete: :cascade
  add_foreign_key "entities_imports", "entities_lists", on_delete: :cascade
  add_foreign_key "entities_lists", "agents", on_delete: :cascade
  add_foreign_key "favorite_agents", "agents", on_delete: :cascade
  add_foreign_key "favorite_agents", "users", on_delete: :cascade
  add_foreign_key "intents", "agents", on_delete: :cascade
  add_foreign_key "interpretation_aliases", "interpretations", on_delete: :cascade
  add_foreign_key "interpretations", "intents", on_delete: :cascade
  add_foreign_key "memberships", "agents", on_delete: :cascade
  add_foreign_key "memberships", "users", on_delete: :cascade
  add_foreign_key "readmes", "agents", on_delete: :cascade
end
