SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: pgcrypto; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS pgcrypto WITH SCHEMA public;


--
-- Name: EXTENSION pgcrypto; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION pgcrypto IS 'cryptographic functions';


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: agent_arcs; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.agent_arcs (
    id bigint NOT NULL,
    source_id uuid,
    target_id uuid
);


--
-- Name: agent_arcs_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.agent_arcs_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: agent_arcs_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.agent_arcs_id_seq OWNED BY public.agent_arcs.id;


--
-- Name: agent_regression_checks; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.agent_regression_checks (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    sentence character varying,
    language character varying,
    now timestamp without time zone,
    agent_id uuid,
    expected jsonb,
    got jsonb,
    state integer DEFAULT 0,
    "position" integer DEFAULT '-1'::integer,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    spellchecking integer
);


--
-- Name: agents; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.agents (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    name character varying,
    description text,
    agentname character varying,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    owner_id uuid,
    color character varying DEFAULT 'black'::character varying,
    image_data text,
    api_token character varying,
    visibility integer DEFAULT 0,
    source_agent jsonb,
    nlp_updated_at timestamp without time zone,
    locales jsonb,
    slug character varying
);


--
-- Name: ar_internal_metadata; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.ar_internal_metadata (
    key character varying NOT NULL,
    value character varying,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL
);


--
-- Name: bots; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.bots (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    name character varying,
    endpoint character varying,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    wip_enabled boolean DEFAULT true
);


--
-- Name: chat_sessions; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.chat_sessions (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    user_id uuid,
    bot_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    locale character varying DEFAULT 'en-US'::character varying
);


--
-- Name: chat_statements; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.chat_statements (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    speaker integer,
    nature integer DEFAULT 0,
    content jsonb,
    chat_session_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL
);


--
-- Name: entities; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.entities (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    solution text,
    auto_solution_enabled boolean DEFAULT true,
    terms jsonb,
    entities_list_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    "position" integer DEFAULT '-1'::integer,
    searchable_terms text,
    case_sensitive boolean DEFAULT false,
    accent_sensitive boolean DEFAULT false
);


--
-- Name: entities_imports; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.entities_imports (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    file_data text,
    mode integer,
    entities_list_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    status integer DEFAULT 0,
    duration integer DEFAULT 0,
    filesize integer DEFAULT 0,
    user_id uuid
);


--
-- Name: entities_lists; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.entities_lists (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    listname character varying,
    "position" integer DEFAULT '-1'::integer,
    color character varying,
    description text,
    visibility integer DEFAULT 0,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    proximity integer DEFAULT 0,
    entities_count integer DEFAULT 0 NOT NULL
);


--
-- Name: favorite_agents; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.favorite_agents (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    user_id uuid,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL
);


--
-- Name: formulation_aliases; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.formulation_aliases (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    aliasname character varying NOT NULL,
    position_start integer,
    position_end integer,
    formulation_id uuid,
    formulation_aliasable_id uuid,
    nature integer DEFAULT 0,
    is_list boolean DEFAULT false,
    any_enabled boolean DEFAULT false,
    formulation_aliasable_type character varying,
    reg_exp text
);


--
-- Name: formulations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.formulations (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    expression character varying,
    interpretation_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    locale character varying,
    "position" integer DEFAULT '-1'::integer,
    keep_order boolean DEFAULT false,
    solution text,
    auto_solution_enabled boolean DEFAULT true,
    proximity integer DEFAULT 3
);


--
-- Name: friendly_id_slugs; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.friendly_id_slugs (
    id bigint NOT NULL,
    slug character varying NOT NULL,
    sluggable_id uuid NOT NULL,
    sluggable_type character varying(50),
    scope character varying,
    created_at timestamp without time zone
);


--
-- Name: friendly_id_slugs_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.friendly_id_slugs_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: friendly_id_slugs_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.friendly_id_slugs_id_seq OWNED BY public.friendly_id_slugs.id;


--
-- Name: interpretations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.interpretations (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    interpretation_name character varying,
    description text,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    "position" integer DEFAULT '-1'::integer,
    color character varying,
    visibility integer DEFAULT 0
);


--
-- Name: memberships; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.memberships (
    id bigint NOT NULL,
    user_id uuid,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    rights character varying DEFAULT 'show'::character varying
);


--
-- Name: memberships_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.memberships_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: memberships_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.memberships_id_seq OWNED BY public.memberships.id;


--
-- Name: readmes; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.readmes (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    content text,
    agent_id uuid,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL
);


--
-- Name: schema_migrations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.schema_migrations (
    version character varying NOT NULL
);


--
-- Name: users; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.users (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    email character varying DEFAULT ''::character varying NOT NULL,
    encrypted_password character varying DEFAULT ''::character varying NOT NULL,
    reset_password_token character varying,
    reset_password_sent_at timestamp without time zone,
    remember_created_at timestamp without time zone,
    sign_in_count integer DEFAULT 0 NOT NULL,
    current_sign_in_at timestamp without time zone,
    last_sign_in_at timestamp without time zone,
    current_sign_in_ip inet,
    last_sign_in_ip inet,
    confirmation_token character varying,
    confirmed_at timestamp without time zone,
    confirmation_sent_at timestamp without time zone,
    unconfirmed_email character varying,
    failed_attempts integer DEFAULT 0 NOT NULL,
    unlock_token character varying,
    locked_at timestamp without time zone,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    admin boolean DEFAULT false,
    invitation_token character varying,
    invitation_created_at timestamp without time zone,
    invitation_sent_at timestamp without time zone,
    invitation_accepted_at timestamp without time zone,
    invitation_limit integer,
    invited_by_type character varying,
    invited_by_id uuid,
    invitations_count integer DEFAULT 0,
    username character varying,
    name character varying,
    bio text,
    image_data text,
    ui_state jsonb DEFAULT '{}'::jsonb,
    quota_enabled boolean DEFAULT true,
    chatbot_enabled boolean DEFAULT false
);


--
-- Name: agent_arcs id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_arcs ALTER COLUMN id SET DEFAULT nextval('public.agent_arcs_id_seq'::regclass);


--
-- Name: friendly_id_slugs id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.friendly_id_slugs ALTER COLUMN id SET DEFAULT nextval('public.friendly_id_slugs_id_seq'::regclass);


--
-- Name: memberships id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.memberships ALTER COLUMN id SET DEFAULT nextval('public.memberships_id_seq'::regclass);


--
-- Name: agent_arcs agent_arcs_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_arcs
    ADD CONSTRAINT agent_arcs_pkey PRIMARY KEY (id);


--
-- Name: agent_regression_checks agent_regression_checks_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_regression_checks
    ADD CONSTRAINT agent_regression_checks_pkey PRIMARY KEY (id);


--
-- Name: agents agents_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agents
    ADD CONSTRAINT agents_pkey PRIMARY KEY (id);


--
-- Name: ar_internal_metadata ar_internal_metadata_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.ar_internal_metadata
    ADD CONSTRAINT ar_internal_metadata_pkey PRIMARY KEY (key);


--
-- Name: bots bots_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.bots
    ADD CONSTRAINT bots_pkey PRIMARY KEY (id);


--
-- Name: chat_sessions chat_sessions_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_sessions
    ADD CONSTRAINT chat_sessions_pkey PRIMARY KEY (id);


--
-- Name: chat_statements chat_statements_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_statements
    ADD CONSTRAINT chat_statements_pkey PRIMARY KEY (id);


--
-- Name: entities_imports entities_imports_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities_imports
    ADD CONSTRAINT entities_imports_pkey PRIMARY KEY (id);


--
-- Name: entities_lists entities_lists_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities_lists
    ADD CONSTRAINT entities_lists_pkey PRIMARY KEY (id);


--
-- Name: entities entities_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities
    ADD CONSTRAINT entities_pkey PRIMARY KEY (id);


--
-- Name: favorite_agents favorite_agents_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.favorite_agents
    ADD CONSTRAINT favorite_agents_pkey PRIMARY KEY (id);


--
-- Name: friendly_id_slugs friendly_id_slugs_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.friendly_id_slugs
    ADD CONSTRAINT friendly_id_slugs_pkey PRIMARY KEY (id);


--
-- Name: entities index_entities_on_entities_list_id_and_position; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities
    ADD CONSTRAINT index_entities_on_entities_list_id_and_position UNIQUE (entities_list_id, "position") DEFERRABLE;


--
-- Name: formulations index_interpretations_on_intent_id_locale_and_position; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.formulations
    ADD CONSTRAINT index_interpretations_on_intent_id_locale_and_position UNIQUE (interpretation_id, locale, "position") DEFERRABLE;


--
-- Name: interpretations intents_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.interpretations
    ADD CONSTRAINT intents_pkey PRIMARY KEY (id);


--
-- Name: formulation_aliases interpretation_aliases_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.formulation_aliases
    ADD CONSTRAINT interpretation_aliases_pkey PRIMARY KEY (id);


--
-- Name: formulations interpretations_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.formulations
    ADD CONSTRAINT interpretations_pkey PRIMARY KEY (id);


--
-- Name: memberships memberships_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.memberships
    ADD CONSTRAINT memberships_pkey PRIMARY KEY (id);


--
-- Name: readmes readmes_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.readmes
    ADD CONSTRAINT readmes_pkey PRIMARY KEY (id);


--
-- Name: schema_migrations schema_migrations_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.schema_migrations
    ADD CONSTRAINT schema_migrations_pkey PRIMARY KEY (version);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: index_agent_arcs_on_source_id_and_target_id; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_agent_arcs_on_source_id_and_target_id ON public.agent_arcs USING btree (source_id, target_id);


--
-- Name: index_agent_regression_checks_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_agent_regression_checks_on_agent_id ON public.agent_regression_checks USING btree (agent_id);


--
-- Name: index_agents_on_api_token; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_agents_on_api_token ON public.agents USING btree (api_token);


--
-- Name: index_agents_on_owner_id_and_agentname; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_agents_on_owner_id_and_agentname ON public.agents USING btree (owner_id, agentname);


--
-- Name: index_bots_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_bots_on_agent_id ON public.bots USING btree (agent_id);


--
-- Name: index_chat_sessions_on_bot_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_chat_sessions_on_bot_id ON public.chat_sessions USING btree (bot_id);


--
-- Name: index_chat_sessions_on_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_chat_sessions_on_user_id ON public.chat_sessions USING btree (user_id);


--
-- Name: index_chat_statements_on_chat_session_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_chat_statements_on_chat_session_id ON public.chat_statements USING btree (chat_session_id);


--
-- Name: index_entities_imports_on_entities_list_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_entities_imports_on_entities_list_id ON public.entities_imports USING btree (entities_list_id);


--
-- Name: index_entities_lists_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_entities_lists_on_agent_id ON public.entities_lists USING btree (agent_id);


--
-- Name: index_entities_on_entities_list_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_entities_on_entities_list_id ON public.entities USING btree (entities_list_id);


--
-- Name: index_entities_on_terms; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_entities_on_terms ON public.entities USING gin (terms);


--
-- Name: index_favorite_agents_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_favorite_agents_on_agent_id ON public.favorite_agents USING btree (agent_id);


--
-- Name: index_favorite_agents_on_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_favorite_agents_on_user_id ON public.favorite_agents USING btree (user_id);


--
-- Name: index_favorite_agents_on_user_id_and_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_favorite_agents_on_user_id_and_agent_id ON public.favorite_agents USING btree (user_id, agent_id);


--
-- Name: index_formulation_aliases_on_formulation_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_formulation_aliases_on_formulation_id ON public.formulation_aliases USING btree (formulation_id);


--
-- Name: index_formulations_on_interpretation_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_formulations_on_interpretation_id ON public.formulations USING btree (interpretation_id);


--
-- Name: index_friendly_id_slugs_on_slug_and_sluggable_type; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_friendly_id_slugs_on_slug_and_sluggable_type ON public.friendly_id_slugs USING btree (slug, sluggable_type);


--
-- Name: index_friendly_id_slugs_on_slug_and_sluggable_type_and_scope; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_friendly_id_slugs_on_slug_and_sluggable_type_and_scope ON public.friendly_id_slugs USING btree (slug, sluggable_type, scope);


--
-- Name: index_friendly_id_slugs_on_sluggable_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_friendly_id_slugs_on_sluggable_id ON public.friendly_id_slugs USING btree (sluggable_id);


--
-- Name: index_friendly_id_slugs_on_sluggable_type; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_friendly_id_slugs_on_sluggable_type ON public.friendly_id_slugs USING btree (sluggable_type);


--
-- Name: index_ialiases_on_ialiasable_type_and_ialiasable_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_ialiases_on_ialiasable_type_and_ialiasable_id ON public.formulation_aliases USING btree (formulation_aliasable_type, formulation_aliasable_id);


--
-- Name: index_interpretations_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_interpretations_on_agent_id ON public.interpretations USING btree (agent_id);


--
-- Name: index_interpretations_on_interpretation_name_and_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_interpretations_on_interpretation_name_and_agent_id ON public.interpretations USING btree (interpretation_name, agent_id);


--
-- Name: index_memberships_on_agent_id_and_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_memberships_on_agent_id_and_user_id ON public.memberships USING btree (agent_id, user_id);


--
-- Name: index_readmes_on_agent_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_readmes_on_agent_id ON public.readmes USING btree (agent_id);


--
-- Name: index_users_on_confirmation_token; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_confirmation_token ON public.users USING btree (confirmation_token);


--
-- Name: index_users_on_email; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_email ON public.users USING btree (email);


--
-- Name: index_users_on_invitation_token; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_invitation_token ON public.users USING btree (invitation_token);


--
-- Name: index_users_on_invitations_count; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_users_on_invitations_count ON public.users USING btree (invitations_count);


--
-- Name: index_users_on_invited_by_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_users_on_invited_by_id ON public.users USING btree (invited_by_id);


--
-- Name: index_users_on_invited_by_type_and_invited_by_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX index_users_on_invited_by_type_and_invited_by_id ON public.users USING btree (invited_by_type, invited_by_id);


--
-- Name: index_users_on_reset_password_token; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_reset_password_token ON public.users USING btree (reset_password_token);


--
-- Name: index_users_on_unlock_token; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_unlock_token ON public.users USING btree (unlock_token);


--
-- Name: index_users_on_username; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX index_users_on_username ON public.users USING btree (username);


--
-- Name: agent_regression_checks fk_rails_0dcfffec3c; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_regression_checks
    ADD CONSTRAINT fk_rails_0dcfffec3c FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: favorite_agents fk_rails_0fcae3d88f; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.favorite_agents
    ADD CONSTRAINT fk_rails_0fcae3d88f FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;


--
-- Name: memberships fk_rails_217f170eb7; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.memberships
    ADD CONSTRAINT fk_rails_217f170eb7 FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: agents fk_rails_2ec9ab8593; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agents
    ADD CONSTRAINT fk_rails_2ec9ab8593 FOREIGN KEY (owner_id) REFERENCES public.users(id);


--
-- Name: agent_arcs fk_rails_468a311c35; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_arcs
    ADD CONSTRAINT fk_rails_468a311c35 FOREIGN KEY (target_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: bots fk_rails_4bd5a3b9b9; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.bots
    ADD CONSTRAINT fk_rails_4bd5a3b9b9 FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: chat_statements fk_rails_655fa33a02; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_statements
    ADD CONSTRAINT fk_rails_655fa33a02 FOREIGN KEY (chat_session_id) REFERENCES public.chat_sessions(id) ON DELETE CASCADE;


--
-- Name: chat_sessions fk_rails_6d2ee2e9b7; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_sessions
    ADD CONSTRAINT fk_rails_6d2ee2e9b7 FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;


--
-- Name: interpretations fk_rails_799ec70975; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.interpretations
    ADD CONSTRAINT fk_rails_799ec70975 FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: entities_lists fk_rails_79b09e3f2b; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities_lists
    ADD CONSTRAINT fk_rails_79b09e3f2b FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: formulations fk_rails_7c761fd9bc; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.formulations
    ADD CONSTRAINT fk_rails_7c761fd9bc FOREIGN KEY (interpretation_id) REFERENCES public.interpretations(id) ON DELETE CASCADE;


--
-- Name: entities_imports fk_rails_88c41ffe66; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities_imports
    ADD CONSTRAINT fk_rails_88c41ffe66 FOREIGN KEY (entities_list_id) REFERENCES public.entities_lists(id) ON DELETE CASCADE;


--
-- Name: agent_arcs fk_rails_95db84923f; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.agent_arcs
    ADD CONSTRAINT fk_rails_95db84923f FOREIGN KEY (source_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: chat_sessions fk_rails_9859487ffe; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.chat_sessions
    ADD CONSTRAINT fk_rails_9859487ffe FOREIGN KEY (bot_id) REFERENCES public.bots(id) ON DELETE CASCADE;


--
-- Name: memberships fk_rails_99326fb65d; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.memberships
    ADD CONSTRAINT fk_rails_99326fb65d FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE;


--
-- Name: readmes fk_rails_9957ac55b4; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.readmes
    ADD CONSTRAINT fk_rails_9957ac55b4 FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- Name: formulation_aliases fk_rails_bed1931875; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.formulation_aliases
    ADD CONSTRAINT fk_rails_bed1931875 FOREIGN KEY (formulation_id) REFERENCES public.formulations(id) ON DELETE CASCADE;


--
-- Name: entities fk_rails_c1fe4c2b97; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.entities
    ADD CONSTRAINT fk_rails_c1fe4c2b97 FOREIGN KEY (entities_list_id) REFERENCES public.entities_lists(id) ON DELETE CASCADE;


--
-- Name: favorite_agents fk_rails_c7c6b86f19; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.favorite_agents
    ADD CONSTRAINT fk_rails_c7c6b86f19 FOREIGN KEY (agent_id) REFERENCES public.agents(id) ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

SET search_path TO "$user", public;

INSERT INTO "schema_migrations" (version) VALUES
('20170912063509'),
('20170912145413'),
('20170918142114'),
('20170920125523'),
('20170926122126'),
('20170927065415'),
('20171009075847'),
('20171009080206'),
('20171009085209'),
('20171009114519'),
('20171009125135'),
('20171012122420'),
('20171013073854'),
('20171016114217'),
('20171019123348'),
('20171020072626'),
('20171023071851'),
('20171030155848'),
('20171106133451'),
('20171106143706'),
('20171107144047'),
('20171108092907'),
('20171109092932'),
('20171113144756'),
('20171114101806'),
('20171115094044'),
('20171116130506'),
('20171122103416'),
('20171124103717'),
('20171124145014'),
('20171127084247'),
('20171128151146'),
('20171128161639'),
('20171129102345'),
('20171129150528'),
('20171129153008'),
('20171201131020'),
('20171206083552'),
('20171206152252'),
('20171207090553'),
('20171211084412'),
('20171212081844'),
('20180103152608'),
('20180108083533'),
('20180123151817'),
('20180126104321'),
('20180206102733'),
('20180208094531'),
('20180313095304'),
('20180314081810'),
('20180321162900'),
('20180322162723'),
('20180406083214'),
('20180411130045'),
('20180417074632'),
('20180417125213'),
('20180418070143'),
('20180418070901'),
('20180504134825'),
('20180516073306'),
('20180516123032'),
('20180827124416'),
('20181023152900'),
('20181121091528'),
('20190116134343'),
('20190130141915'),
('20190226153941'),
('20190228101317'),
('20190228142502'),
('20190301133440'),
('20190305084159'),
('20190311102618'),
('20190312150519'),
('20190322134748'),
('20190322143442'),
('20190322144120'),
('20190403120508'),
('20190405120340'),
('20190408083049'),
('20190412074505'),
('20190424102329'),
('20190503151110'),
('20190517140227'),
('20190619122021'),
('20190729072037'),
('20190729112306'),
('20190925090036'),
('20191018130844'),
('20191025090323'),
('20191028091856'),
('20191029091355'),
('20191029145256');
