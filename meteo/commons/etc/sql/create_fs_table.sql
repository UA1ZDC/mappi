CREATE TABLE meteo.@tablename@ (
    id bigint NOT NULL,
    filename text NOT NULL,
    filemd5 text NOT NULL,
    filesize integer NOT NULL,
    dt_write timestamp without time zone DEFAULT timezone('utc'::text, now()) NOT NULL
);


ALTER TABLE meteo.@tablename@ OWNER TO postgres;

--
-- Name: TABLE fs; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.@tablename@ IS 'Описание файлов, на которые могут ссылаться другие записи';


--
-- Name: fs_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.@tablename@_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.@tablename@_id_seq OWNER TO postgres;

--
-- Name: fs_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.@tablename@_id_seq OWNED BY meteo.@tablename@.id;


--
-- Name: fs id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.@tablename@ ALTER COLUMN id SET DEFAULT nextval('meteo.@tablename@_id_seq'::regclass);


--
-- Name: fs filename_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.@tablename@
    ADD CONSTRAINT @tablename@_filename_uniq UNIQUE (filename);


--
-- Name: fs fs_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.@tablename@
    ADD CONSTRAINT @tablename@_pkey PRIMARY KEY (id);
